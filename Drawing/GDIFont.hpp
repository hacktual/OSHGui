#ifndef OSHGUI_DRAWING_GDIFONT_HPP
#define OSHGUI_DRAWING_GDIFONT_HPP

#include "Font.hpp"
#include "Image.hpp"

#include <vector>
#include <Windows.h>

#undef DrawText

namespace OSHGui
{
	namespace Drawing
	{
		class OSHGUI_EXPORT GDIFont : public Font
		{
		public:
			GDIFont(const Misc::AnsiString &faceName, const float pointSize, const bool antiAliased, const Effect effect, const float lineSpacing = 0.0f);

			~GDIFont();

			float GetPointSize() const;

			bool IsAntiAliased() const;

			void SetPointSize(const float pointSize);

			void SetAntiAliased(const bool antiAliased);

		protected:
			void DrawGlyphToBuffer(uint32_t *buffer, uint32_t width) const;

			unsigned int GetTextureSize(CodepointIterator start, CodepointIterator end) const;

			void Free();

			void InitialiseFontGlyph(CodepointIterator cp) const;

			void InitialiseGlyphMap();

			virtual const FontGlyph* FindFontGlyph(const uint32_t codepoint) const override;
			virtual void Rasterise(uint32_t start_codepoint, uint32_t end_codepoint) const override;
			virtual void UpdateFont() override;

			Misc::AnsiString faceName;
			float lineSpacing;
			float pointSize;
			bool antiAliased;
			HDC context;
			HFONT font;
			TEXTMETRICA textMetric;
			uint32_t* bitmapBits;
			HBITMAP bitmap;
			typedef std::vector<TexturePtr> TextureVector;
			mutable TextureVector glyphTextures;
			typedef std::vector<ImagePtr> ImageVector;
			mutable ImageVector glyphImages;
		};
	}
}

#endif
