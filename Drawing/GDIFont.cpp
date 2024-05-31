#include "GDIFont.hpp"
#include "Texture.hpp"
#include "../Application.hpp"
#include "../Misc/Exceptions.hpp"

#undef DrawText

namespace OSHGui
{
	namespace Drawing
	{
		const unsigned int GlyphPadding = 2;

		GDIFont::GDIFont(const Misc::AnsiString &_faceName, const float _pointSize, const bool _antiAliased, const Effect _effect, const float _lineSpacing)
			: faceName(_faceName),
			  lineSpacing(_lineSpacing),
			  pointSize(_pointSize),
			  antiAliased(_antiAliased),
			  context(nullptr),
			  font(nullptr),
			  bitmap(nullptr)
		{
			effect = _effect;

			UpdateFont();
		}

		GDIFont::~GDIFont()
		{
			Free();
		}

		float GDIFont::GetPointSize() const
		{
			return pointSize;
		}

		void GDIFont::SetPointSize(const float _pointSize)
		{
			if (pointSize == _pointSize)
			{
				return;
			}

			pointSize = _pointSize;

			UpdateFont();
		}

		bool GDIFont::IsAntiAliased() const
		{
			return antiAliased;
		}

		void GDIFont::SetAntiAliased(const bool _antiAliasing)
		{
			if (antiAliased == _antiAliasing)
			{
				return;
			}

			antiAliased = _antiAliasing;

			UpdateFont();
		}

		const FontGlyph* GDIFont::FindFontGlyph(const uint32_t codepoint) const
		{
			auto it = glyphMap.find(codepoint);
			if (it == glyphMap.end())
			{
				return nullptr;
			}

			if (!it->second.IsValid())
			{
				InitialiseFontGlyph(it);
			}

			return &it->second;
		}

		uint32_t GDIFont::GetTextureSize(CodepointIterator start, CodepointIterator end) const
		{
			auto size = 32;
			const auto maximum = Application::Instance().GetRenderer().GetMaximumTextureSize();
			auto count = 0;

			while (size < maximum)
			{
				auto x = GlyphPadding;
				auto y = GlyphPadding;
				auto yb = GlyphPadding;
				for (auto c = start; c != end; ++c)
				{
					if (c->second.GetImage())
					{
						continue;
					}

					const auto glyphWidth = static_cast<int>(std::ceil(textMetric.tmMaxCharWidth)) + GlyphPadding + static_cast<uint32_t>(effect);
					const auto glyphHeight = static_cast<int>(std::ceil(textMetric.tmHeight)) + GlyphPadding + static_cast<uint32_t>(effect);

					x += glyphWidth;
					if (x > size)
					{
						x = GlyphPadding;
						y = yb;
					}
					const auto yy = y + glyphHeight;
					if (yy > size)
					{
						goto too_small;
					}

					if (yy > yb)
					{
						yb = yy;
					}

					++count;
				}
				break;

				too_small:
				size *= 2;
			}

			return count ? size : 0;
		}

		void GDIFont::Rasterise(uint32_t startCodepoint, uint32_t endCodepoint) const
		{
			auto start = glyphMap.lower_bound(startCodepoint);
			if (start == glyphMap.end())
			{
				return;
			}

			const auto bck = start;
			const auto end = glyphMap.upper_bound(endCodepoint);
			while (true)
			{
				const auto textureSize = GetTextureSize(start, end);
				if (textureSize == 0)
				{
					break;
				}

				auto texture = Application::Instance().GetRenderer().CreateTexture(SizeF(textureSize, textureSize));
				glyphTextures.push_back(texture);

				std::vector<uint32_t> buffer(textureSize * textureSize);
				
				auto x = GlyphPadding;
				auto y = GlyphPadding;
				auto yb = GlyphPadding;

				bool finished = false;
				bool forward = true;

				while (start != glyphMap.end())
				{
					finished |= (start == end);

					if (!start->second.GetImage())
					{
						const RECT area = { 0, 0, textMetric.tmMaxCharWidth, textMetric.tmHeight };
						const auto codepoint = static_cast<wchar_t>(start->first);

						if (!ExtTextOutW(context, 0, 0, ETO_OPAQUE, &area, &codepoint, 1, nullptr))
						{
							const auto image = std::make_shared<Image>(texture, RectangleF(0, 0, 0, 0), PointF(0, 0));
							glyphImages.push_back(image);
							start->second.SetImage(image);
						}
						else
						{
							const auto glyphWidth = textMetric.tmMaxCharWidth + GlyphPadding + static_cast<uint32_t>(effect);
							const auto glyphHeight = textMetric.tmHeight + GlyphPadding + static_cast<uint32_t>(effect);

							auto next = x + glyphWidth;
							if (next > textureSize)
							{
								x = GlyphPadding;
								next = x + glyphWidth;
								y = yb;
							}

							const auto bottom = y + glyphHeight;
							if (bottom > textureSize)
							{
								break;
							}

							DrawGlyphToBuffer(buffer.data() + (y * textureSize) + x, textureSize);

							RectangleF area(x, y, glyphWidth - GlyphPadding, glyphHeight - GlyphPadding);
							PointF offset(0, static_cast<float>(-ascender));

							const auto image = std::make_shared<Image>(texture, area, offset);
							glyphImages.push_back(image);
							start->second.SetImage(image);

							x = next;
							if (bottom > yb)
							{
								yb = bottom;
							}
						}
					}

					if (forward)
					{
						if (++start == glyphMap.end())
						{
							finished = true;
							forward = false;
							start = bck;
						}
					}
					if (!forward)
					{
						if ((start == glyphMap.begin()) || (--start == glyphMap.begin()))
						{
							break;
						}
					}
				}

				ApplyEffect(buffer.data(), textureSize, textureSize);

				texture->LoadFromMemory(buffer.data(), SizeF(textureSize, textureSize), Texture::PixelFormat::RGBA);

				if (finished)
				{
					break;
				}
			}
		}

		void GDIFont::DrawGlyphToBuffer(uint32_t *buffer, uint32_t width) const
		{
			for (auto i = 0; i < textMetric.tmHeight; ++i)
			{
				for (auto j = 0; j < textMetric.tmMaxCharWidth; ++j)
				{
					const auto green = static_cast<uint8_t>((bitmapBits[i * textMetric.tmMaxCharWidth + j] >> 8) & 0xFF);
					buffer[j] = 0x00FFFFFF | (green << 24);
				}

				buffer += width;
			}
		}

		void GDIFont::Free()
		{
			if (!context)
			{
				return;
			}

			glyphMap.clear();
			glyphImages.clear();
			glyphTextures.clear();

			DeleteDC(context);
			DeleteObject(font);
			DeleteObject(bitmap);
		}

		void GDIFont::UpdateFont()
		{
			Free();

			context = CreateCompatibleDC(nullptr);
			if (!context)
			{
				throw Misc::Exception();
			}

			SetMapMode(context, MM_TEXT);
			SetTextAlign(context, TA_LEFT | TA_TOP);
			SetBkColor(context, TRANSPARENT);
			SetTextColor(context, RGB(255, 255, 255));

			const auto dpiVertical = static_cast<uint32_t>(Application::Instance().GetRenderer().GetDisplayDPI().Y);

			font = CreateFontA(-MulDiv(static_cast<int>(pointSize), dpiVertical, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, antiAliased ? CLEARTYPE_NATURAL_QUALITY : NONANTIALIASED_QUALITY, DEFAULT_PITCH, faceName.c_str());
			if (!font)
			{
				DeleteDC(context);

				throw Misc::Exception();
			}

			SelectObject(context, font);

			if (!GetTextMetricsA(context, &textMetric))
			{
				DeleteDC(context);
				DeleteObject(font);

				throw Misc::Exception();
			}

			BITMAPINFO bmi;
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = textMetric.tmMaxCharWidth;
			bmi.bmiHeader.biHeight = -textMetric.tmHeight;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biBitCount = 32;

			bitmap = CreateDIBSection(context, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&bitmapBits), nullptr, 0);
			if (!bitmap)
			{
				DeleteDC(context);
				DeleteObject(font);

				throw Misc::Exception();
			}

			SelectObject(context, bitmap);

			ascender = static_cast<float>(textMetric.tmAscent);
			descender = static_cast<float>(-textMetric.tmDescent);
			height = static_cast<float>(textMetric.tmHeight);

			if (lineSpacing > 0.0f)
			{
				height = lineSpacing;
			}

			InitialiseGlyphMap();
		}

		void GDIFont::InitialiseGlyphMap()
		{
			const auto setSize = GetFontUnicodeRanges(context, nullptr);
			if (!setSize)
			{
				throw Misc::Exception();
			}

			std::vector<uint8_t> buffer(setSize);
			auto set = reinterpret_cast<GLYPHSET*>(buffer.data());

			if (!GetFontUnicodeRanges(context, set))
			{
				throw Misc::Exception();
			}

			auto maximum = 0;

			for (auto i = 0; i < set->cRanges; ++i)
			{
				const auto range = set->ranges[i];
				auto codepoint = range.wcLow + range.cGlyphs - 1;

				if (maximum < codepoint)
				{
					maximum = codepoint;
				}

				while (codepoint >= range.wcLow)
				{
					glyphMap[codepoint] = FontGlyph();

					codepoint--;
				}
			}

			SetMaxCodepoint(maximum);
		}

		void GDIFont::InitialiseFontGlyph(CodepointIterator it) const
		{
			ABC abc;
			if (!GetCharABCWidthsW(context, it->first, it->first, &abc))
			{
				return;
			}

			const auto advance = static_cast<float>(abc.abcA + abc.abcB + abc.abcC);

			it->second.SetAdvance(advance);
			it->second.SetValid(true);
		}
	}
}
