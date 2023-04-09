#ifndef SQUI_FONTSTORE_HPP
#define SQUI_FONTSTORE_HPP

#include "ft2build.h"
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
				float advance{};

				std::tuple<Quad, vec2, float> getQuad(const std::shared_ptr<ID3D11ShaderResourceView> &textureView) {
					return {Quad(Quad::Args{
						.size = size,
						.texture = textureView,
						.textureType = TextureType::Text,
						.textureUv = {
							uvTopLeft.x, uvTopLeft.y,
							uvBottomRight.x, uvBottomRight.y,
						},
					}), offset, advance};
				}
			};
		private:

            std::string fontPath;
			Atlas atlas{};
			//
            std::unordered_map<float, std::unordered_map<char32_t, CharInfo>> chars{};

		public:
			bool loaded{true};
			FT_Face face{};
            Font(std::string fontPath);

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

			/**
			 * @brief Gets the quad for a character.
			 * Will return an empty quad if the character can't be generated.
			 * 
			 * @param character The character to get the quad for
			 * @param size The size of the font
			 * @return std::tuple<Quad, vec2, float> 
			 */
			std::tuple<Quad, vec2, float> getCharQuad(unsigned char *character, float size);

			/**
			 * @brief Gets the CharInfo for a character.
			 * 
			 * @param character The character to get the CharInfo for
			 * @param size The size of the font
			 * @return CharInfo& 
			 */
			CharInfo &getCharInfo(unsigned char *character, float size);

			Atlas &getAtlas();

			void updateTexture();
		};

        // TODO: refcount fonts and delete when no longer used

		static FT_Library ftLibrary;
		static std::unordered_map<std::string, Font> fonts;

    public:
		static std::tuple<uint32_t, uint32_t> getTextSize(std::string_view text, const std::string &fontPath, float size);
	
        static std::tuple<std::vector<std::vector<Quad>>, float, float> generateQuads(std::string_view text, const std::string &fontPath, float size, const vec2 &pos, const Color &color, const float &maxWidth = -1.0f);
	};
}// namespace squi


#endif