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
			struct CharInfo {
				vec2 uvTopLeft{};
				vec2 uvBottomRight{};
				vec2 size{};

				Quad getQuad(const std::shared_ptr<ID3D11ShaderResourceView> &textureView) {
					return Quad(Quad::Args{
						.size = size,
						.texture = textureView,
						.textureType = Quad::TextureType::Text,
						.textureUv = {
							uvTopLeft.x, uvTopLeft.y,
							uvBottomRight.x, uvBottomRight.y,
						},
					});
				}
			};

            std::string fontPath;
			Atlas atlas{};
			//
            std::unordered_map<float, std::unordered_map<char32_t, CharInfo>> chars{};

		public:
			bool loaded{true};
			FT_Face face{};
            Font(std::string fontPath);

			Quad getQuad(unsigned char *character, float size);

			void updateTexture();
		};

        // TODO: refcount fonts and delete when no longer used

		static FT_Library ftLibrary;
		static std::unordered_map<std::string, Font> fonts;

    public:
        static std::vector<Quad> generateQuads(std::string text, const std::string &fontPath, float size, const vec2 &pos, const Color &color);
	};
}// namespace squi


#endif