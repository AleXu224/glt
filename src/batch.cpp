#include "batch.hpp"

using namespace squi;

Batch::Batch() {
	// vertices.fill(Vertex{{0, 0, 0}});
	// indices.fill(0);
	
	// Vertex array
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// Vertex buffer
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	const auto stride = sizeof(Vertex);
	uintptr_t pos = 0;
	glBufferData(GL_ARRAY_BUFFER, VERTEX_BATCH * stride, vertices.data(), GL_DYNAMIC_DRAW);
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
	glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, stride, (void *) pos);
	glEnableVertexAttribArray(7);
	// pos += sizeof(float);

	// Index buffer
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_BATCH * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
}

void Batch::freeBuffers() {
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

std::tuple<std::span<Vertex>, std::span<unsigned int>, unsigned int>
Batch::addVertex() {
	unsigned int index;
	if (!freeVertexes.empty()) {
		index = freeVertexes.back();
		freeVertexes.pop_back();
	} else {
		index = firstFreeVertex;
		firstFreeVertex++;
	}

	return {
		std::span<Vertex>(vertices).subspan(index * 4, 4),
		std::span<unsigned int>(indices).subspan(index * 6, 6),
		index,
	};
}

void Batch::removeVertex(unsigned int index) {
	if (index == firstFreeVertex - 1) {
		firstFreeVertex--;
	} else {
		freeVertexes.push_back(index);
	}

	// 0 fill indices
	const unsigned int offset = index * 6;
	indices[offset] = 0;
	indices[offset + 1] = 0;
	indices[offset + 2] = 0;
	indices[offset + 3] = 0;
	indices[offset + 4] = 0;
	indices[offset + 5] = 0;

}

bool Batch::isFull() const {
	return firstFreeVertex == VERTEX_BATCH / 4 && freeVertexes.empty();
}

bool Batch::isEmpty() const {
	if (firstFreeVertex == 0 && freeVertexes.empty()) {
		return true;
	} else if (firstFreeVertex == freeVertexes.size()) {
		return true;
	} else {
		return false;
	}
}

void Batch::render() {
	glBindVertexArray(vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VERTEX_BATCH * sizeof(Vertex), vertices.data());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, INDEX_BATCH * sizeof(unsigned int), indices.data());

	// glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);	

	glDrawElements(GL_TRIANGLES, INDEX_BATCH, GL_UNSIGNED_INT, nullptr);
	// glDrawArrays(GL_TRIANGLES, 0, 3);
}