#ifndef SQUI_FONTSTORE_HPP
#define SQUI_FONTSTORE_HPP

#define GL_WITH_GLAD
#include "freetype-gl/freetype-gl.h"
#include "memory"
#include "unordered_map"
#include "quad.hpp"
#include "string"

namespace squi {
	class FontStore {
		class Font {
            std::string fontPath;
			std::shared_ptr<texture_atlas_t> atlas;
            std::unordered_map<float, std::shared_ptr<texture_font_t>> sizes;

		public:
            Font(std::string fontPath);

            std::shared_ptr<texture_font_t> getSize(float size);
		};

        // TODO: refcount fonts and delete when no longer used

		static std::unordered_map<std::string, Font> fonts;

    public:
        static std::shared_ptr<texture_font_t> getFont(std::string fontPath, float size);

        static std::vector<Quad> generateQuads(std::string text, std::string fontPath, float size, glm::vec2 pos, glm::vec4 color);
	};
}// namespace squi


#endif