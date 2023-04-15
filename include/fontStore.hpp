#ifndef SQUI_FONTSTORE_HPP
#define SQUI_FONTSTORE_HPP

#include "ft2build.h"
#include <freetype/fttypes.h>
#include <tuple>
#include FT_FREETYPE_H
#include "DirectXMath.h"
#include "memory"
#include "unordered_map"
#include "unordered_set"
#include "quad.hpp"
#include "string"
#include "atlas.hpp"
#include "color.hpp"

namespace squi {
	class FontStore {
		class Font {
		public:
			struct CharInfo {
				vec2 uvTopLeft{};
				vec2 uvBottomRight{};
				vec2 size{};
				vec2 offset{};
				int32_t advance{};
				FT_UInt index{};

				std::unordered_map<char32_t, int32_t> kerning{};

				int32_t getKerning(const FT_Face &face, const FT_UInt &prevIndex) {
					if (const auto &it = kerning.find(prevIndex); it != kerning.end()) {
						return it->second;
					}

					FT_Vector kerning;
					FT_Get_Kerning(face, prevIndex, index, FT_KERNING_DEFAULT, &kerning);
					this->kerning[prevIndex] = kerning.x >> 6;
					return kerning.x;
				}
			};
		private:

			Atlas atlas{};
			//
            std::unordered_map<float, std::unordered_map<char32_t, CharInfo>> chars{};

		public:
			bool loaded{true};
			FT_Face face{};
            explicit Font(std::string_view fontPath);

			/**
			 * @brief Generates the texture for a character and adds it to the atlas. 
			 * There is no additional cost if the texture has already been generated.
			 * 
			 * @param character The character to generate the texture for
			 * @param size The size of the font
			 * @return true 
			 * @return false 
			 */
			bool generateTexture(unsigned char *character, float size);
			bool generateTexture(unsigned char *character, std::unordered_map<char32_t, CharInfo> &sizeMap);

			/**
			 * @brief Gets the CharInfo for a character.
			 * 
			 * @param character The character to get the CharInfo for
			 * @param size The size of the font
			 * @return CharInfo& 
			 */
			CharInfo &getCharInfo(unsigned char *character, float size);
			CharInfo &getCharInfo(unsigned char *character, std::unordered_map<char32_t, CharInfo> &sizeMap);

			Atlas &getAtlas();

			std::unordered_map<char32_t, CharInfo> &getSizeMap(float size);

			void updateTexture();
		};

        // TODO: refcount fonts and delete when no longer used

		static FT_Library ftLibrary;
		static std::unordered_map<std::string_view, Font> fonts;

    public:
		/**
		 * @brief Get the Height of a font at a given size.
		 * 
		 * @param fontPath 
		 * @param size 
		 * @return uint32_t 
		 */
		static uint32_t getLineHeight(std::string_view fontPath, float size);
	
		/**
		 * @brief Get the Width and Height of a string of text.
		 * 
		 * @param text 
		 * @param fontPath 
		 * @param size 
		 * @return std::tuple<uint32_t, uint32_t> 
		 */
		static std::tuple<uint32_t, uint32_t> getTextSize(std::string_view text, std::string_view fontPath, float size);
	
		/**
		 * @brief Generates a 2d vector of quads for a string of text.
		 * 
		 * Each row in the vector represents a line of text.
		 * 
		 * @param text 
		 * @param fontPath 
		 * @param size 
		 * @param pos 
		 * @param color 
		 * @param maxWidth The max width of the text before wrapping. A value of -1 means no wrapping.
		 * @return std::tuple<std::vector<std::vector<Quad>> quads, float width, float height> 
		 */
        static std::tuple<std::vector<std::vector<Quad>>, float, float> generateQuads(std::string_view text, std::string_view fontPath, float size, const vec2 &pos, const Color &color, const float &maxWidth = -1.0f);
	};
}// namespace squi


#endif