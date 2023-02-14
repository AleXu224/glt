#ifndef SQUI_BATCH_HPP
#define SQUI_BATCH_HPP

#include "array"
#include "glad/glad.h"
#include "span"
#include "vector"
#include "vertex.hpp"

constexpr unsigned int VERTEX_BATCH = 1000 * 4;
constexpr unsigned int INDEX_BATCH = 1000 * 6;

constexpr size_t BATCH_SIZE = 1024;

namespace squi {
	class Batch {
		GLuint vbo;
		GLuint ebo;
		GLuint vao;

		std::array<Vertex, VERTEX_BATCH> vertices{};
		std::array<unsigned int, INDEX_BATCH> indices{};

		// On adding a vertex:
		// 1. Check if there are free vertexes in freeVertexes and
		// use the first one then remove it from freeVertexes
		// 2. If there are no free vertexes then use the firstFreeVertex
		// and increment it by one

		// On removing a vertex:
		// If the vertex is the last one then decrement firstFreeVertex
		// Else add the vertex to freeVertexes
		unsigned int firstFreeVertex = 0;
		std::vector<unsigned int> freeVertexes{};

	public:
		Batch();

		std::tuple<std::span<Vertex>, std::span<unsigned int>, unsigned int>
		addVertex();

		void removeVertex(unsigned int index);

		bool isFull() const;

		bool isEmpty() const;

		void render();

		void freeBuffers();
	};

	template<typename T>
		requires std::is_base_of_v<VertexBase, T>
	class QuadBatch {
		GLuint vao;
		GLuint vbo;
		GLuint ebo;

		std::vector<T> vertices{};
		std::vector<unsigned int> indices{};

	public:
		QuadBatch() {
			std::tie(vao, vbo) = T().generateBuffers(BATCH_SIZE);
		}

		
	};
}// namespace squi

#endif