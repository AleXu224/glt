#ifndef SQUI_VERTEX_HPP
#define SQUI_VERTEX_HPP
#include "DirectXMath.h"
#include "tuple"

namespace squi {
	struct Vertex {
		/**
		 * @brief UV coordinates of the vertex.
		 * Should always be 0,0 for the top left corner and 1,1 for the bottom right corner
		 */
		DirectX::XMFLOAT2 uv;
		/**
		 * @brief UV coordinates for the texture.
		 */
		DirectX::XMFLOAT2 texUv;
		/**
		 * @brief Internally used by the shader to find its respective data.
		 */
		uint32_t id;
	};

	enum class TextureType : uint32_t {
		NoTexture = 0,
		Texture = 1,
		Text = 2
	};

	// TODO: switch to 16 bit floats
	struct VertexData {
		/**
		 * @brief Inner color of the quad
		 */
		DirectX::XMFLOAT4 color;
		/**
		 * @brief Border color of the quad
		 */
		DirectX::XMFLOAT4 borderColor;
		/**
		 * @brief Screen position of the quad (top left corner)
		 */
		DirectX::XMFLOAT2 pos;
		/**
		 * @brief Size of the quad
		 */
		DirectX::XMFLOAT2 size;
		/**
		 * @brief Used for text. This offset is given for all characters in a string of text
		 * so that they all have the same position
		 */
		DirectX::XMFLOAT2 offset;
		/**
		 * @brief Radius of the border
		 */
		float borderRadius;
		/**
		 * @brief Size of the border
		 */
		float borderSize;
		/**
		 * @brief Texture id of the quad. 0 if no texture is used
		 */
		uint32_t textureIndex;
		TextureType textureType;
		DirectX::XMFLOAT4 clipRect;
		float clipBorderRadius;
	};
}// namespace squi


#endif