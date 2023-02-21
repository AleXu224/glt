#ifndef SQUI_VERTEX_HPP
#define SQUI_VERTEX_HPP
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "tuple"

namespace squi {
	struct Vertex {
		/**
		 * @brief UV coordinates of the vertex.
		 * Should always be 0,0 for the top left corner and 1,1 for the bottom right corner
		 */
		glm::vec2 uv;
		/**
		 * @brief UV coordinates for the texture.
		 */
		glm::vec2 texUv;
		/**
		 * @brief Internally used by the shader to find its respective data.
		 */
		glm::u32 id;
	};

	struct VertexData {
		/**
		 * @brief Inner color of the quad
		 */
		glm::vec4 color;
		/**
		 * @brief Border color of the quad
		 */
		glm::vec4 borderColor;
		/**
		 * @brief Screen position of the quad (top left corner)
		 */
		glm::vec2 pos;
		/**
		 * @brief Size of the quad
		 */
		glm::vec2 size;
		/**
		 * @brief Used for text. This offset is given for all characters in a string of text
		 * so that they all have the same position
		 */
		glm::vec2 offset;
		/**
		 * @brief Radius of the border
		 */
		glm::float32 borderRadius;
		/**
		 * @brief Size of the border
		 */
		glm::float32 borderSize;
		/**
		 * @brief Texture id of the quad. 0 if no texture is used
		 */
		glm::float32 textureId;
		glm::vec3 padding; // Padding to make the struct 16 byte aligned
	};
}// namespace squi


#endif