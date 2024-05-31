/*
 * OldSchoolHack GUI
 *
 * by KN4CK3R https://www.oldschoolhack.me/
 *
 * See license in OSHGui.hpp
 */

#include "FontManager.hpp"
#include "../Misc/Exceptions.hpp"
#include "../Misc/HashTuple.hpp"
#include "FreeTypeFont.hpp"
#include "GDIFont.hpp"
#include <windows.h>
#include <algorithm>
#include <vector>
#include <sstream>

namespace OSHGui
{
	namespace Drawing
	{
		std::unordered_map<std::tuple<Misc::AnsiString, float, bool, Font::Effect>, std::weak_ptr<Drawing::Font>> FontManager::loadedFonts;
		//---------------------------------------------------------------------------
		FontPtr FontManager::LoadFreeTypeFont(Misc::AnsiString name, float pointSize, bool antiAliased, Font::Effect effect)
		{
			if (name.empty())
			{
				throw Misc::ArgumentException();
			}

			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			std::vector<std::string> parts;
			std::stringstream ss(name);
			for (std::string s; ss >> s; parts.push_back(s));

			FontPtr font = nullptr;

			char keyNameBuffer[MAX_PATH];
			char keyValueBuffer[MAX_PATH];

			HKEY fontKey;
			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &fontKey) == ERROR_SUCCESS)
			{
				int i = 0;
				int lastError = ERROR_SUCCESS;
				do
				{
					DWORD keyNameLength = sizeof(keyNameBuffer);
					DWORD keyValueLength = sizeof(keyValueBuffer);

					lastError = RegEnumValueA(fontKey, i, keyNameBuffer, &keyNameLength, nullptr, nullptr, reinterpret_cast<uint8_t*>(keyValueBuffer), &keyValueLength);
					if (lastError == ERROR_SUCCESS)
					{
						std::string keyName(keyNameBuffer);
						if (keyName.find(" (TrueType)") != std::string::npos || keyName.find(" (OpenType)") != std::string::npos)
						{
							keyName.erase(keyName.length() - 11);
						}

						if (keyName.length() == name.length())
						{
							std::transform(keyName.begin(), keyName.end(), keyName.begin(), ::tolower);

							auto foundAll = true;
							for (auto &part : parts)
							{
								if (keyName.find(part) == std::string::npos)
								{
									foundAll = false;
									break;
								}
							}

							if (foundAll)
							{
								char path[MAX_PATH] = { };
								ExpandEnvironmentStringsA((std::string("%windir%\\fonts\\") + keyValueBuffer).c_str(), path, sizeof(path));

								RegCloseKey(fontKey);

								return LoadFreeTypeFontFromFile(path, pointSize, antiAliased, effect);
							}
						}
					}

					++i;
				} while (lastError != ERROR_NO_MORE_ITEMS);

				RegCloseKey(fontKey);
			}

			throw Misc::ArgumentException();
		}
		//---------------------------------------------------------------------------
		FontPtr FontManager::LoadFreeTypeFontFromFile(const Misc::AnsiString &filename, float pointSize, bool antiAliased, Font::Effect effect)
		{
			auto cacheEntry = std::make_tuple(filename, pointSize, antiAliased, effect);
			const auto it = loadedFonts.find(cacheEntry);
			if (it == std::end(loadedFonts) || it->second.expired())
			{
				auto font = std::make_shared<FreeTypeFont>(filename, pointSize, antiAliased, effect);
				loadedFonts[cacheEntry] = font;
				return font;
			}
			return it->second.lock();
		}
		//---------------------------------------------------------------------------
		FontPtr FontManager::LoadFreeTypeFontFromMemory(const Misc::RawDataContainer &data, float pointSize, bool antiAliased, Font::Effect effect)
		{
			return std::make_shared<FreeTypeFont>(data, pointSize, antiAliased, effect);
		}
		//---------------------------------------------------------------------------
		void FontManager::DisplaySizeChanged(const SizeF &size)
		{
			for (auto &font : loadedFonts)
			{
				font.second.lock()->DisplaySizeChanged(size);
			}
		}
		//---------------------------------------------------------------------------
		FontPtr FontManager::LoadGDIFont(Misc::AnsiString name, float pointSize, bool antiAliased, Font::Effect effect)
		{
			return std::make_shared<GDIFont>(name, pointSize, antiAliased, effect);
		}
		//---------------------------------------------------------------------------
	}
}
