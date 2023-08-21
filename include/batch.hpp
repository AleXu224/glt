#pragma once

#include "array"
#include "quad.hpp"
#include "shader.hpp"
#include "vector"
#include "vertex.hpp"


constexpr size_t BATCH_SIZE = 1000;
constexpr size_t VERTEX_BATCH = BATCH_SIZE * 4;
constexpr size_t INDEX_BATCH = BATCH_SIZE * 6;

namespace squi {
	class Batch {
		std::shared_ptr<ID3D11Buffer> vertexBuffer;
		std::shared_ptr<ID3D11Buffer> indexBuffer;
		std::shared_ptr<ID3D11Buffer> structuredBuffer;
		std::shared_ptr<ID3D11ShaderResourceView> structuredBufferView;
		std::shared_ptr<ID3D11SamplerState> samplerState;

		std::array<Vertex, VERTEX_BATCH> vertices{};
		std::array<VertexData, BATCH_SIZE> data{};
		std::array<uint32_t, INDEX_BATCH> indices{};
		std::vector<std::shared_ptr<ID3D11ShaderResourceView>> textures{};
		uint32_t maxTextureCount = 16;// 16 is the minimum required by OpenGL

		size_t cursor = 0;

	public:
		Batch(const std::shared_ptr<ID3D11Device> &device);

		void addQuad(Quad &quad,
					 Shader &shader,
					 std::shared_ptr<ID3D11DeviceContext> &context,
					 std::shared_ptr<ID3D11RenderTargetView> &renderTargetView,
					 D3D11_VIEWPORT &viewport);

		void render(Shader &shader,
					const std::shared_ptr<ID3D11DeviceContext> &context,
					const std::shared_ptr<ID3D11RenderTargetView> &renderTargetView,
					const D3D11_VIEWPORT &viewport);
	};
}// namespace squi