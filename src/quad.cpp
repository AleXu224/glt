#include "quad.hpp"

using namespace squi;

Quad::Quad(const Args &args)
	: vertices{
		  Vertex{
			  .uv = {0.0f, 0.0f},
			  .texUv = {args.textureUv.x, args.textureUv.y},
			  .id = 0,
		  },
		  Vertex{
			  .uv = {1.0f, 0.0f},
			  .texUv = {args.textureUv.z, args.textureUv.y},
			  .id = 0,
		  },
		  Vertex{
			  .uv = {1.0f, 1.0f},
			  .texUv = {args.textureUv.z, args.textureUv.w},
			  .id = 0,
		  },
		  Vertex{
			  .uv = {0.0f, 1.0f},
			  .texUv = {args.textureUv.x, args.textureUv.w},
			  .id = 0,
		  },
	  },
	  data{
		  .color = args.color,
		  .borderColor = args.borderColor,
		  .pos = args.pos,
		  .size = args.size,
		  .offset = args.offset,
		  .borderRadius = args.borderRadius,
		  .borderSize = args.borderSize,
		  .textureIndex = 0,
		  .textureType = args.textureType,
		  .clipRect = {0.0f, 0.0f, 0.0f, 0.0f},
		  .clipBorderRadius = 0.0f,
	  },
	  texture{args.texture} {}

void Quad::setId(uint32_t id) {
	vertices[0].id = id;
	vertices[1].id = id;
	vertices[2].id = id;
	vertices[3].id = id;
}

void Quad::setTextureIndex(uint32_t id) {
	data.textureIndex = id;
}

const TextureType &Quad::getTextureType() const {
	return data.textureType;
}

const std::shared_ptr<ID3D11ShaderResourceView> &Quad::getTexture() const {
	return texture;
}

const std::array<Vertex, 4> &Quad::getVertices() const {
	return vertices;
}

const VertexData &Quad::getData() const {
	return data;
}

void Quad::setPos(DirectX::XMFLOAT2 pos) {
	data.pos = pos;
}

void Quad::setSize(DirectX::XMFLOAT2 size) {
	data.size = size;
}

void Quad::setOffset(DirectX::XMFLOAT2 offset) {
	data.offset = offset;
}

void Quad::setColor(DirectX::XMFLOAT4 color) {
	data.color = color;
}

void Quad::setBorderColor(DirectX::XMFLOAT4 color) {
	data.borderColor = color;
}

void Quad::setBorderRadius(float radius) {
	data.borderRadius = radius;
}

void Quad::setBorderSize(float size) {
	data.borderSize = size;
}

void Quad::setClipRect(const Rect &clipRect, float clipBorderRadius) {
	data.clipRect = DirectX::XMFLOAT4(clipRect.left, clipRect.top, clipRect.right, clipRect.bottom);
	data.clipBorderRadius = clipBorderRadius;
}