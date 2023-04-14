#ifndef SQUI_SQUARE_HPP
#define SQUI_SQUARE_HPP
#include "array"
#include "d3d11.h"
#include "memory"
#include "rect.hpp"
#include "span"
#include "vertex.hpp"

namespace squi {
	class Quad {
	private:
		std::array<Vertex, 4> vertices;
		VertexData data;
		std::shared_ptr<ID3D11ShaderResourceView> texture;

	public:
		struct Args {
			DirectX::XMFLOAT2 pos;
			DirectX::XMFLOAT2 size;
			DirectX::XMFLOAT2 offset;
			DirectX::XMFLOAT4 color = {1.0f, 1.0f, 1.0f, 1.0f};
			DirectX::XMFLOAT4 borderColor = {0.0f, 0.0f, 0.0f, 1.0f};
			float borderRadius;
			float borderSize;
			std::shared_ptr<ID3D11ShaderResourceView> texture;
			TextureType textureType = TextureType::NoTexture;
			DirectX::XMFLOAT4 textureUv;
		};

		Quad(const Args &args);

		[[nodiscard]] const std::array<Vertex, 4> &getVertices() const;
		[[nodiscard]] const VertexData &getData() const;

		void setId(uint32_t id);

		void setTextureIndex(uint32_t id);
		[[nodiscard]] const TextureType &getTextureType() const;
		[[nodiscard]] const std::shared_ptr<ID3D11ShaderResourceView> &getTexture() const;

		void setPos(DirectX::XMFLOAT2 pos);
		void setSize(DirectX::XMFLOAT2 size);
		void setOffset(DirectX::XMFLOAT2 offset);
		void setColor(DirectX::XMFLOAT4 color);
		void setBorderColor(DirectX::XMFLOAT4 color);
		void setBorderRadius(float radius);
		void setBorderSize(float size);
		void setClipRect(const Rect &clipRect, float clipBorderRadius);
	};
}// namespace squi

#endif