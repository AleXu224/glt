#include "batch.hpp"

using namespace squi;

Batch::Batch() {
	// Vertex array
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// Vertex buffer
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	const auto stride = sizeof(Vertex);
	uintptr_t pos = 0;
	glBufferData(GL_ARRAY_BUFFER, VERTEX_BATCH * stride, vertices.data(), GL_DYNAMIC_DRAW);

	// UV
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *) pos);
	glEnableVertexAttribArray(0);
	pos += sizeof(glm::vec2);
	// texUV
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *) pos);
	glEnableVertexAttribArray(1);
	pos += sizeof(glm::vec2);
	// ID
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, stride, (void *) pos);
	glEnableVertexAttribArray(2);
	// pos += sizeof(uint16_t);

	// Index buffer
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_BATCH * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

	// SSBO buffer
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, BATCH_SIZE * sizeof(VertexData), nullptr, GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Batch::freeBuffers() {
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void Batch::addQuad(Quad &quad) {
	if (cursor >= BATCH_SIZE) {
		render();
	}
	if (quad.getTextureType() != Quad::TextureType::NoTexture) {
		const auto &textureId = quad.getTextureId();
		const auto iter = std::find(textures.begin(), textures.end(), textureId);
		if (iter == textures.end()) {
			if (textures.size() >= maxTextureCount) {
				render();
			}
			glBindTextureUnit(textures.size(), textureId);
			quad.setTextureIndex(textures.size());
			textures.push_back(textureId);
		} else {
			quad.setTextureIndex(std::distance(textures.begin(), iter));
		}
	}
	quad.setId(cursor);
	auto quadVertices = quad.getVertices();
	const auto verticesOffset = cursor * 4;
	vertices[verticesOffset] = quadVertices[0];
	vertices[verticesOffset + 1] = quadVertices[1];
	vertices[verticesOffset + 2] = quadVertices[2];
	vertices[verticesOffset + 3] = quadVertices[3];

	const auto indicesOffset = cursor * 6;
	indices[indicesOffset] = verticesOffset;
	indices[indicesOffset + 1] = verticesOffset + 1;
	indices[indicesOffset + 2] = verticesOffset + 2;
	indices[indicesOffset + 3] = verticesOffset + 0;
	indices[indicesOffset + 4] = verticesOffset + 2;
	indices[indicesOffset + 5] = verticesOffset + 3;

	data[cursor++] = quad.getData();
}

void Batch::render() {
	if (cursor == 0) {
		return;
	}

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VERTEX_BATCH * sizeof(Vertex), vertices.data());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, INDEX_BATCH * sizeof(unsigned int), indices.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(VertexData) * BATCH_SIZE, data.data());

	glDrawElements(GL_TRIANGLES, INDEX_BATCH, GL_UNSIGNED_INT, nullptr);

	cursor = 0;
	indices.fill(0);
	textures.clear();
}