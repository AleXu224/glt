#ifndef SQUI_BATCH_HPP
#define SQUI_BATCH_HPP

#include "array"
#include "glad/glad.h"
#include "span"
#include "vector"
#include "unordered_map"
#include "vertex.hpp"
#include "quad.hpp"

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

	// The QuadBatch should reuse the same buffers and data inside and only update said data
	// It should also be able to return a quad when requested
	template<typename T>
		requires std::is_base_of_v<QuadBase, T>
	class QuadBatch {
		std::vector<GLuint> vaos{};
		std::vector<GLuint> vbos{};
		std::vector<GLuint> ebos{};

		std::vector<std::array<T, BATCH_SIZE>> vertices{};
		std::vector<std::array<unsigned int, BATCH_SIZE * 6>> indices{};

		size_t quadCount = 0;

		void pushBuffer() {
			GLuint vao, vbo, ebo;
			std::tie(vao, vbo) = T().generateBuffers(BATCH_SIZE);
			glGenBuffers(1, &ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * BATCH_SIZE * 6, nullptr, GL_DYNAMIC_DRAW);

			vaos.push_back(vao);
			vbos.push_back(vbo);
			ebos.push_back(ebo);

			vertices.push_back({});
			indices.push_back({});
		}

		void popBuffer() {
			glDeleteBuffers(1, &vaos.back());
			glDeleteBuffers(1, &vbos.back());
			glDeleteBuffers(1, &ebos.back());

			vaos.pop_back();
			vbos.pop_back();
			ebos.pop_back();
		}

		/**
		 * @brief Get a span of all the vertexes in the batch
		 * 
		 * @return std::span<T> 
		 */
		std::span<T> getVertexes() {
			return {&vertices.at(0).front(), vertices.size() * BATCH_SIZE};
		}

		std::span<T> getIndices() {
			return {&indices.at(0).front(), indices.size() * BATCH_SIZE * 6};
		}
		
	public:
		void add(T quad) {
			if (++quadCount > vertices.size() * BATCH_SIZE) {
				pushBuffer();
			}

			size_t index = quadCount - 1;

			vertices.back()[index % BATCH_SIZE] = quad;
			std::span<unsigned int> indicesSpan = {&indices.back()[(index % BATCH_SIZE) * 6], 6};
			size_t indiceIndex = (index % BATCH_SIZE) * 4;
			indicesSpan[0] = indiceIndex;
			indicesSpan[1] = indiceIndex + 1;
			indicesSpan[2] = indiceIndex + 2;
			indicesSpan[3] = indiceIndex;
			indicesSpan[4] = indiceIndex + 2;
			indicesSpan[5] = indiceIndex + 3;

			std::ranges::sort(getVertexes(), [](const T& a, const T& b) {
				return a < b;
			});
		}

		void generateIndexBuffer() {
			auto verticesSpan = getVertexes();
			auto indicesSpan = getIndices();
		}
	};
}// namespace squi

#endif