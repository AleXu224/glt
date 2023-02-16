#ifndef SQUI_VERTEX_HPP
#define SQUI_VERTEX_HPP
#include "glm/glm.hpp"
#include "glad/glad.h"
#include "tuple"

namespace squi {
	struct Vertex {
		glm::vec2 position;
		glm::vec4 color;
		glm::vec2 uv;
		glm::vec2 size;
		glm::float32 borderRadius;
		glm::float32 borderSize;
		glm::vec4 borderColor;
		glm::float32 z;
	};
}// namespace squi


#endif