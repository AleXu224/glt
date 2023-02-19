#ifndef SQUI_VERTEX_HPP
#define SQUI_VERTEX_HPP
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "tuple"

namespace squi {
	struct Vertex {
		glm::vec2 uv;
		glm::vec2 texUv;
		glm::float32 id;
	};
}// namespace squi


#endif