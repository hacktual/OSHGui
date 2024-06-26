/*
 * OldSchoolHack GUI
 *
 * by KN4CK3R https://www.oldschoolhack.me/
 *
 * See license in OSHGui.hpp
 */

#ifndef OSHGUI_DRAWING_FONTMANAGER_HPP
#define OSHGUI_DRAWING_FONTMANAGER_HPP

#include <unordered_map>
#include "../Exports.hpp"
#include "Font.hpp"
#include "../Misc/RawDataContainer.hpp"

namespace OSHGui
{
	namespace Drawing
	{
		/**
		 * Die Klasse ermöglicht das Laden einer Schriftart.
		 */
		class OSHGUI_EXPORT FontManager
		{
		public:
			/**
			 * Lädt die Schriftart mit dem entsprechenden Namen.
			 *
			 * \param name Name der Schriftart
			 * \param pointSize Größe in PT
			 * \param antiAliased Legt fest, ob AntiAliasing verwendet werden soll
			 * \return Die geladene Schriftart oder nullptr, falls die Schriftart nicht gefunden wird.
			 */
			static FontPtr LoadFreeTypeFont(Misc::AnsiString name, float pointSize, bool antiAliased, Font::Effect effect = Font::Effect::NONE);
			/**
			 * Lädt die Schriftart aus der angegebenen Datei.
			 *
			 * \param file Pfad zur Datei
			 * \param pointSize Größe in PT
			 * \param antiAliased Legt fest, ob AntiAliasing verwendet werden soll
			 * \return Die geladene Schriftart oder nullptr, falls die Schriftart nicht gefunden wird.
			 */
			static FontPtr LoadFreeTypeFontFromFile(const Misc::AnsiString &file, float pointSize, bool antiAliased, Font::Effect effect = Font::Effect::NONE);
			static FontPtr LoadFreeTypeFontFromMemory(const Misc::RawDataContainer &data, float pointSize, bool antiAliased, Font::Effect effect = Font::Effect::NONE);

			/**
			* Legt die Display-Größe fest.
			*
			* @param size
			*/
			static void DisplaySizeChanged(const SizeF &size);

			static FontPtr LoadGDIFont(Misc::AnsiString name, float pointSize, bool antiAliased, Font::Effect effect = Font::Effect::NONE);

		private:
			static std::unordered_map<std::tuple<Misc::AnsiString, float, bool, Font::Effect>, std::weak_ptr<Drawing::Font>> loadedFonts;
		};
	}
}

#endif