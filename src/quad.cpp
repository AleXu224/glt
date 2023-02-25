#include "quad.hpp"
#include "renderer.hpp"

using namespace squi;

Quad::Quad(const Args &args) {
	for (auto &vertex: vertices) {
		vertex.id = 0;
	}

	vertices[0].uv = {0.0f, 0.0f};
	vertices[1].uv = {1.0f, 0.0f};
	vertices[2].uv = {1.0f, 1.0f};
	vertices[3].uv = {0.0f, 1.0f};

	const auto &texUv = args.textureUv;
	vertices[0].texUv = {texUv.x, texUv.y};
	vertices[1].texUv = {texUv.z, texUv.y};
	vertices[2].texUv = {texUv.z, texUv.w};
	vertices[3].texUv = {texUv.x, texUv.w};

	data.color = args.color;
	data.borderColor = args.borderColor;
	data.pos = args.pos;
	data.size = args.size;
	data.offset = args.offset;
	data.borderRadius = args.borderRadius;
	data.borderSize = args.borderSize;
	textureId = args.textureId;
	data.textureType = static_cast<uint32_t>(args.textureType);
	// if (args.textureType != TextureType::NoTexture) {
	// 	data.textureIndex = glGetTextureHandleARB(textureId);
	// }
}

void Quad::setId(int id) {
	for (auto &vertex: vertices) {
		vertex.id = id;
	}
}

void Quad::setTextureIndex(int id) {
	data.textureIndex = id;
}

Quad::TextureType Quad::getTextureType() const {
	return static_cast<const TextureType>(data.textureType);
}

const GLuint& Quad::getTextureId() const {
	return textureId;
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