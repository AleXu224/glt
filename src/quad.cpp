#include "quad.hpp"
#include "renderer.hpp"

using namespace squi;

Quad::Quad(glm::vec2 position,
		   glm::vec2 size,
		   glm::vec4 color,
		   float borderRadius,
		   float borderSize,
		   glm::vec4 borderColor,
		   float z) {
	auto &renderer = *Renderer::get();

	std::tie(vertices, indices, batchIndex, vertexIndex) = renderer.addVertex();

	vertices[0].position = {position.x, position.y};
	vertices[1].position = {position.x + size.x, position.y};
	vertices[2].position = {position.x + size.x, position.y + size.y};
	vertices[3].position = {position.x, position.y + size.y};

	for (auto &vertex: vertices) {
		vertex.color = color;
		vertex.size = size;
		vertex.borderRadius = borderRadius;
        vertex.borderSize = borderSize;
        vertex.borderColor = borderColor;
		vertex.z = z;
	}

	vertices[0].uv = {0.0f, 0.0f};
	vertices[1].uv = {1.0f, 0.0f};
	vertices[2].uv = {1.0f, 1.0f};
	vertices[3].uv = {0.0f, 1.0f};

	const unsigned int indiceIndex = vertexIndex * 4;
	indices[0] = indiceIndex;
	indices[1] = indiceIndex + 1;
	indices[2] = indiceIndex + 2;
	indices[3] = indiceIndex;
	indices[4] = indiceIndex + 2;
	indices[5] = indiceIndex + 3;
}

void Quad::free() {
	Renderer::get()->removeVertex(batchIndex, vertexIndex);
}