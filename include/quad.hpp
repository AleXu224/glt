#ifndef SQUI_SQUARE_HPP
#define SQUI_SQUARE_HPP
#include "span"
#include "array"
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

	/**
	 * @brief Base class for vertex types
	 * 
	 */
	struct QuadBase {
		/**
		 * @brief Generate vertex array and vertex buffer
		 * 
		 * @return std::tuple<GLuint VAO, GLuint VBO> 
		 */
		virtual std::tuple<GLuint, GLuint> generateBuffers(size_t size) const = 0;

		virtual inline unsigned int getIndex() const = 0;

		virtual bool operator<(const QuadBase &other) const = 0;
	};

	/**
	 * @brief Vertex type for a rounded box with a border
	 * 
	 */
	struct BoxQuad : public QuadBase {
		struct Vertex {
			glm::vec2 pos;
			glm::vec4 color;
			glm::vec2 uv;
			glm::vec2 size;
			glm::float32 borderRadius;
			glm::float32 borderSize;
			glm::vec4 borderColor;
			glm::u32 z = std::numeric_limits<uint32_t>::max();
		};
		std::array<Vertex, 4> vertices;


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
			glBufferData(GL_ARRAY_BUFFER, size * sizeof(BoxQuad), nullptr, GL_DYNAMIC_DRAW);
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
			return vertices[0].z;
		}

		bool operator<(const QuadBase &other) const override {
			return vertices[0].z < other.getIndex();
		}
	};
}// namespace squi

#endif