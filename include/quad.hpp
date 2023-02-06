#ifndef SQUI_SQUARE_HPP
#define SQUI_SQUARE_HPP
#include "span"
#include "vertex.hpp"

namespace squi {
	class Quad {
		std::span<Vertex> vertices;
		std::span<unsigned int> indices;
		unsigned int batchIndex;
		unsigned int vertexIndex;

	public:
		Quad(
			glm::vec2 position,
			glm::vec2 size,
			glm::vec4 color,
			float borderRadius = 0.0f,
            float borderSize = 0.0f,
            glm::vec4 borderColor = {0.0f, 0.0f, 0.0f, 1.0f},
			float z = 0.0f);
		void free();
	};
}// namespace squi

#endif