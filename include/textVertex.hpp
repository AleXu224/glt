#ifndef SQUI_TEXTVERTEX_HPP
#define SQUI_TEXTVERTEX_HPP

#include "glm/glm.hpp"

namespace squi {
	struct TextVertex {
		glm::vec2 position{};
		glm::vec2 texCoord{};
		glm::vec2 size{};
		glm::vec4 color{};
	};
}

#endif