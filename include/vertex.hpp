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

	/**
	 * @brief Base class for vertex types
	 * 
	 */
	struct VertexBase {
		/**
		 * @brief Generate vertex array and vertex buffer
		 * 
		 * @return std::tuple<GLuint VAO, GLuint VBO> 
		 */
		virtual std::tuple<GLuint, GLuint> generateBuffers(size_t size) const = 0;

		virtual inline unsigned int getIndex() const = 0;

		virtual bool operator<(const VertexBase &other) const = 0;
	};

	/**
	 * @brief Vertex type for a rounded box with a border
	 * 
	 */
	struct BoxVertex : public VertexBase {
		glm::vec2 pos;
		glm::vec4 color;
		glm::vec2 uv;
		glm::vec2 size;
		glm::float32 borderRadius;
		glm::float32 borderSize;
		glm::vec4 borderColor;
		glm::u32 z;

		std::tuple<GLuint, GLuint> generateBuffers(size_t size) const override {
			GLuint vao, vbo;
			
			// Vertex array
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			// Vertex buffer
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			const auto stride = sizeof(Vertex);
			uintptr_t pos = 0;
			glBufferData(GL_ARRAY_BUFFER, size * sizeof(BoxVertex), nullptr, GL_DYNAMIC_DRAW);
			// Pos
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *) pos);
			glEnableVertexAttribArray(0);
			pos += sizeof(glm::vec2);
			// Color
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void *) pos);
			glEnableVertexAttribArray(1);
			pos += sizeof(glm::vec4);
			// UV
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *) pos);
			glEnableVertexAttribArray(2);
			pos += sizeof(glm::vec2);
			// Size
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void *) pos);
			glEnableVertexAttribArray(3);
			pos += sizeof(glm::vec2);
			// Border radius
			glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void *) pos);
			glEnableVertexAttribArray(4);
			pos += sizeof(float);
			// Border size
			glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride, (void *) pos);
			glEnableVertexAttribArray(5);
			pos += sizeof(float);
			// Border color
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void *) pos);
			glEnableVertexAttribArray(6);
			pos += sizeof(glm::vec4);
			// Z
			glVertexAttribPointer(7, 1, GL_UNSIGNED_INT, GL_FALSE, stride, (void *) pos);
			glEnableVertexAttribArray(7);
			// pos += sizeof(float);

			return {vao, vbo};
		}

		inline unsigned int getIndex() const override {
			return z;
		}

		bool operator<(const VertexBase &other) const override {
			return z < other.getIndex();
		}
	};
}// namespace squi


#endif