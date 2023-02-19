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

	data[0] = {color.r, color.g, color.b, color.a};
	data[1] = {borderColor.r, borderColor.g, borderColor.b, borderColor.a};
	data[2] = {position.x, position.y, size.x, size.y};
	data[3] = {borderRadius, borderSize, 0.0f, 0.0f};
}

void Quad::setId(int id) {
	for (auto &vertex: vertices) {
		vertex.id = id;
	}
}

void Quad::setTextureId(int id) {
	data[3].z = id;
}

std::span<const Vertex> Quad::getVertices() const {
	return vertices;
}

const glm::mat4 &Quad::getData() const {
	return data;
}

void Quad::setPos(glm::vec2 pos) {
	data[2].x = pos.x;
	data[2].y = pos.y;
}

void Quad::setSize(glm::vec2 size) {
	data[2].z = size.x;
	data[2].w = size.y;
}

void Quad::setColor(glm::vec4 color) {
	data[0] = {color.r, color.g, color.b, color.a};
}

void Quad::setBorderColor(glm::vec4 color) {
	data[1] = {color.r, color.g, color.b, color.a};
}

void Quad::setBorderRadius(float radius) {
	data[3].x = radius;
}

void Quad::setBorderSize(float size) {
	data[3].y = size;
}