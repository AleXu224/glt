#include "quad.hpp"
#include "renderer.hpp"

using namespace squi;

Quad::Quad(glm::vec2 position,
		   glm::vec2 size,
		   glm::vec4 color,
		   float borderRadius,
		   float borderSize,
		   glm::vec4 borderColor) {

	for (auto &vertex: vertices) {
		vertex.id = 0;
	}

	vertices[0].uv = {0.0f, 0.0f};
	vertices[1].uv = {1.0f, 0.0f};
	vertices[2].uv = {1.0f, 1.0f};
	vertices[3].uv = {0.0f, 1.0f};

	vertices[0].texUv = {0.0f, 0.0f};
	vertices[1].texUv = {1.0f, 0.0f};
	vertices[2].texUv = {1.0f, 1.0f};
	vertices[3].texUv = {0.0f, 1.0f};

	data.color = color;
	data.borderColor = borderColor;
	data.pos = position;
	data.size = size;
	data.offset = {0.0f, 0.0f};
	data.borderRadius = borderRadius;
	data.borderSize = borderSize;
	data.textureId = 0;
}

void Quad::setId(int id) {
	for (auto &vertex: vertices) {
		vertex.id = id;
	}
}

void Quad::setTextureId(int id) {
	data.textureId = id;
}

std::span<const Vertex> Quad::getVertices() const {
	return vertices;
}

const VertexData &Quad::getData() const {
	return data;
}

void Quad::setPos(glm::vec2 pos) {
	data.pos = pos;
}

void Quad::setSize(glm::vec2 size) {
	data.size = size;
}

void Quad::setColor(glm::vec4 color) {
	data.color = color;
}

void Quad::setBorderColor(glm::vec4 color) {
	data.borderColor = color;
}

void Quad::setBorderRadius(float radius) {
	data.borderRadius = radius;
}

void Quad::setBorderSize(float size) {
	data.borderSize = size;
}