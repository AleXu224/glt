#ifndef SQUI_TEXTBATCH_HPP
#define SQUI_TEXTBATCH_HPP

#define GL_WITH_GLAD
#include "freetype-gl/freetype-gl.h"
#include "memory"
#include "string"
#include "glm/glm.hpp"
#include "array"
#include "vector"
#include "span"
#include "textVertex.hpp"

namespace squi {
    constexpr auto TEXT_BATCH = 1024;

    class TextBatch {
        std::shared_ptr<texture_atlas_t> atlas;
        std::shared_ptr<texture_font_t> font;
        std::array<TextVertex, TEXT_BATCH * 4> quads{};
        std::array<GLuint, TEXT_BATCH * 6> indices{};
        GLuint vbo;
        GLuint ebo;
        GLuint vao;
        std::string fontPath;

        unsigned int firstFreeQuad = 0;
        std::vector<unsigned int> freeQuads{};

        /**
         * @brief 
         * 
         * @return The id of the quad that was reserved
         */
        unsigned int reserveQuad();

        /**
         * @brief Frees a quad so it can be reused
         * 
         * @param id The id of the quad to free
         */
        void freeQuad(unsigned int id);

        /**
         * @brief Get the vertexes of a given quad id
         * 
         * @param id the quad id
         * @return A span of the vertexes of the quad
         */
        std::span<TextVertex> getQuad(unsigned int id);

    public:
        TextBatch(std::string fontPath);
        void freeBuffers();

        /**
         * Creates the quads for the text at the specified position
         * 
         * @param text The text to create the quads for
         * @param position The position of the text
         * @param size The size of the text
         * @param color The color of the text
         *
         * @return A vector of quad ids that can be retrieved later
         *
         */
		std::vector<unsigned int> createQuads(std::string text, glm::vec2 position, float size, glm::vec4 color);

        /**
         * @brief Renders the text batch
         * 
         */
        void render();
	};
}

#endif