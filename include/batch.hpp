#ifndef SQUI_BATCH_HPP
#define SQUI_BATCH_HPP

#include "array"
#include "glad/glad.h"
#include "quad.hpp"
#include "span"
#include "unordered_map"
#include "vector"
#include "vertex.hpp"

constexpr size_t BATCH_SIZE = 1000;
constexpr size_t VERTEX_BATCH = BATCH_SIZE * 4;
constexpr size_t INDEX_BATCH = BATCH_SIZE * 6;

namespace squi {
	class Batch {
		GLuint vbo;
		GLuint ebo;
		GLuint vao;
		GLuint ssbo;

		std::array<Vertex, VERTEX_BATCH> vertices{};
		std::array<VertexData, BATCH_SIZE> data{};
		std::array<unsigned int, INDEX_BATCH> indices{};

		size_t cursor = 0;

	public:
		Batch();

		void addQuad(Quad &quad);

		void render();

		void freeBuffers();
	};
}// namespace squi

#endif