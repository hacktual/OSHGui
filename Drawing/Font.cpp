#include "Font.hpp"
#include "../Application.hpp"
#include <algorithm>

#undef DrawText

namespace OSHGui
{
	namespace Drawing
	{
		//---------------------------------------------------------------------------
		//static attributes
		//---------------------------------------------------------------------------
		const auto BitsPerUnit = sizeof(uint32_t) * 8;
		const auto GlyphsPerPage = 256;
		//---------------------------------------------------------------------------
		//Constructor
		//---------------------------------------------------------------------------
		Font::Font()
			: ascender(0.0f),
			  descender(0.0f),
			  height(0.0f),
			  scalingHorizontal(1.0f),
			  scalingVertical(1.0f),
			  maximumCodepoint(0),
			  effect(Effect::NONE)
		{
			
		}
		//---------------------------------------------------------------------------
		Font::~Font()
		{
			
		}
		//---------------------------------------------------------------------------
		//Getter/Setter
		//---------------------------------------------------------------------------
		void Font::SetMaxCodepoint(uint32_t codepoint)
		{
			loadedGlyphPages.clear();

			maximumCodepoint = codepoint;

			const auto pages = (codepoint + GlyphsPerPage) / GlyphsPerPage;
			const auto size = (pages + BitsPerUnit - 1) / BitsPerUnit;

			loadedGlyphPages.resize(size * sizeof(uint32_t));
		}
		//---------------------------------------------------------------------------
		const FontGlyph* Font::GetGlyphData(uint32_t codepoint) const
		{
			if (codepoint > maximumCodepoint)
			{
				return nullptr;
			}

			const auto glyph = FindFontGlyph(codepoint);

			if (!loadedGlyphPages.empty())
			{
				const auto page = codepoint / GlyphsPerPage;
				const auto mask = 1 << (page & (BitsPerUnit - 1));
				if (!(loadedGlyphPages[page / BitsPerUnit] & mask))
				{
					loadedGlyphPages[page / BitsPerUnit] |= mask;
					Rasterise(codepoint & ~(GlyphsPerPage - 1), codepoint | (GlyphsPerPage - 1));
				}
			}

			return glyph;
		}
		//---------------------------------------------------------------------------
		const FontGlyph* Font::FindFontGlyph(const uint32_t codepoint) const
		{
			auto pos = glyphMap.find(codepoint);
			return pos != glyphMap.end() ? &pos->second : nullptr;
		}
		//---------------------------------------------------------------------------
		void Font::ApplyEffect(uint32_t* buffer, uint32_t height, uint32_t width) const
		{
			if (effect == Effect::NONE)
			{
				return;
			}

			std::vector<uint32_t> temp(height * width);
			switch (effect)
			{
			case Effect::DROPSHADOW:
				for (auto i = 0; i < height; ++i)
				{
					for (auto j = 0; j < width; ++j)
					{
						const auto alpha = static_cast<uint8_t>((buffer[i * width + j] >> 24) & 0xFF);
						if (!alpha)
						{
							continue;
						}

						temp[(i + 1) * width + j + 1] = 0x00000000 | ((alpha / 2) << 24);
						temp[i * width + j] = buffer[i * width + j];
					}
				}
				break;
			case Effect::OUTLINE:
				for (auto i = 0; i < height; ++i)
				{
					for (auto j = 0; j < width; ++j)
					{
						const auto alpha = static_cast<uint8_t>((buffer[i * width + j] >> 24) & 0xFF);
						if (!alpha)
						{
							continue;
						}

						for (auto k = -1; k < 2; ++k)
						{
							for (auto l = -1; l < 2; ++l)
							{
								temp[(i + 1 + k) * width + j + 1 + l] = 0xFF000000;
							}
						}
					}
				}

				for (auto i = 0; i < height; ++i)
				{
					for (auto j = 0; j < width; ++j)
					{
						const auto alpha = static_cast<uint8_t>((buffer[i * width + j] >> 24) & 0xFF);
						if (!alpha)
						{
							continue;
						}

						temp[(i + 1) * width + j + 1] = buffer[i * width + j];
					}
				}
				break;
			}

			std::memcpy(buffer, temp.data(), height * sizeof(uint32_t) * width);
		}
		//---------------------------------------------------------------------------
		float Font::GetTextExtent(const Misc::AnsiString &text, float scaleX) const
		{
			return GetTextAdvance(text, scaleX);
		}
		//---------------------------------------------------------------------------
		float Font::GetTextAdvance(const Misc::AnsiString &text, float scaleX) const
		{
			float advance = 0.0f;

			for (auto c : text)
			{
				if (const auto glyph = GetGlyphData(static_cast<unsigned char>(c)))
				{
					advance += glyph->GetAdvance(scaleX);
				}
			}

			return advance;
		}
		//---------------------------------------------------------------------------
		size_t Font::GetCharAtPixel(const Misc::AnsiString &text, size_t start, float pixel, float scaleX) const
		{
			auto current = 0.f;
			const auto length = text.length();

			if (pixel <= 0.f || length <= start)
			{
				return start;
			}

			for (auto c = start; c < length; ++c)
			{
				if (const auto glyph = GetGlyphData(static_cast<unsigned char>(text[c])))
				{
					current += glyph->GetAdvance(scaleX);

					if (pixel < current)
					{
						return c;
					}
				}
			}

			return length;
		}
		//---------------------------------------------------------------------------
		//Runtime-Functions
		//---------------------------------------------------------------------------
		void Font::DisplaySizeChanged(const SizeF &size)
		{
			//Image::ComputeScalingFactors(size, d_nativeResolution, scalingHorizontal, scalingVertical);

			UpdateFont();
		}
		//---------------------------------------------------------------------------
		Font::Effect Font::GetEffect() const
		{
			return effect;
		}
		//---------------------------------------------------------------------------
		void Font::SetEffect(const Effect _effect)
		{
			if (effect == _effect)
			{
				return;
			}

			effect = _effect;

			UpdateFont();
		}
		//---------------------------------------------------------------------------
		float Font::DrawText(GeometryBuffer &buffer, const Misc::AnsiString &text, const PointF &position, const RectangleF *clip, const ColorRectangle &colors, const float spaceExtra, const float scaleX, const float scaleY) const
		{
			const auto base = position.Y + GetBaseline(scaleY);
			auto glyphPosition(position);

			for (auto c : text)
			{
				if (const auto glyph = GetGlyphData(static_cast<uint32_t>(c)))
				{
					auto image = glyph->GetImage();
					glyphPosition.Y = base - (image->GetOffset().Y - image->GetOffset().Y * scaleY);
					image->Render(buffer, RectangleF(glyphPosition, glyph->GetSize(scaleX, scaleY)), clip, colors);
					glyphPosition.X += glyph->GetAdvance(scaleX);// - 1.f;

					if (c == ' ')
					{
						glyphPosition.X += spaceExtra;
					}
				}
			}

			return glyphPosition.X;
		}
		//---------------------------------------------------------------------------
		void Font::Rasterise(uint32_t startCodepoint, uint32_t endCodepoint) const
		{
			
		}
		//---------------------------------------------------------------------------
	}
}