#include "batch.hpp"

using namespace squi;

Batch::Batch(const std::shared_ptr<ID3D11Device>& device) {
	// Vertex array
	D3D11_BUFFER_DESC VBufferDesc = {};
	VBufferDesc.ByteWidth = sizeof(vertices);
	VBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	VBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VBufferDesc.MiscFlags = 0;
	VBufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA VBufferResource = {};
	VBufferResource.pSysMem = vertices.data();
	VBufferResource.SysMemPitch = 0;
	VBufferResource.SysMemSlicePitch = 0;

	ID3D11Buffer *VBufferPtr = nullptr;
	device->CreateBuffer(&VBufferDesc, &VBufferResource, &VBufferPtr);
	vertexBuffer.reset(VBufferPtr, [](ID3D11Buffer *bufferPtr) {
		bufferPtr->Release();
	});

	// Index array
	D3D11_BUFFER_DESC IBufferDesc = {};
	IBufferDesc.ByteWidth = sizeof(indices);
	IBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	IBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	IBufferDesc.MiscFlags = 0;
	IBufferDesc.StructureByteStride = sizeof(uint32_t);

	D3D11_SUBRESOURCE_DATA IBufferResource = {};
	IBufferResource.pSysMem = indices.data();
	IBufferResource.SysMemPitch = 0;
	IBufferResource.SysMemSlicePitch = 0;

	ID3D11Buffer *IBufferPtr = nullptr;
	device->CreateBuffer(&IBufferDesc, &IBufferResource, &IBufferPtr);
	indexBuffer.reset(IBufferPtr, [](ID3D11Buffer *bufferPtr) {
		bufferPtr->Release();
	});

	// Structured buffer
	D3D11_BUFFER_DESC SBufferDesc = {};
	SBufferDesc.ByteWidth = sizeof(data);
	SBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	SBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	SBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	SBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	SBufferDesc.StructureByteStride = sizeof(VertexData);

	D3D11_SUBRESOURCE_DATA SBufferResource = {};
	SBufferResource.pSysMem = data.data();

	ID3D11Buffer *SBufferPtr = nullptr;
	device->CreateBuffer(&SBufferDesc, &SBufferResource, &SBufferPtr);

	D3D11_SHADER_RESOURCE_VIEW_DESC SBufferViewDesc = {};
	SBufferViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	SBufferViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SBufferViewDesc.Buffer.FirstElement = 0;
	SBufferViewDesc.Buffer.NumElements = BATCH_SIZE;

	ID3D11ShaderResourceView *SBufferViewPtr = nullptr;
	device->CreateShaderResourceView(SBufferPtr, &SBufferViewDesc, &SBufferViewPtr);
	structuredBuffer.reset(SBufferPtr, [](ID3D11Buffer *bufferPtr) {
		bufferPtr->Release();
	});
	structuredBufferView.reset(SBufferViewPtr, [](ID3D11ShaderResourceView *viewPtr) {
		viewPtr->Release();
	});

	// Sampler state for a alpha only texture
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState *samplerStatePtr = nullptr;
	device->CreateSamplerState(&samplerDesc, &samplerStatePtr);
	samplerState.reset(samplerStatePtr, [](ID3D11SamplerState *statePtr) {
		statePtr->Release();
	});
}

void Batch::addQuad(Quad &quad,
					Shader &shader,
					std::shared_ptr<ID3D11DeviceContext> &context,
					std::shared_ptr<ID3D11RenderTargetView> &renderTargetView,
					D3D11_VIEWPORT &viewport) {
	if (cursor >= BATCH_SIZE) {
		render(shader, context, renderTargetView, viewport);
	}
	if (quad.getTextureType() != Quad::TextureType::NoTexture) {
		const auto &texture = quad.getTexture();
		const auto iter = std::find(textures.begin(), textures.end(), texture);
		if (iter == textures.end()) {
			if (textures.size() >= maxTextureCount) {
				render(shader, context, renderTargetView, viewport);
			}
			quad.setTextureIndex(textures.size());
			textures.push_back(texture);
		} else {
			quad.setTextureIndex(std::distance(textures.begin(), iter));
		}
	}
	quad.setId(cursor);
	const auto &quadVertices = quad.getVertices();
	const auto verticesOffset = cursor * 4;
	vertices[verticesOffset + 0] = quadVertices[0];
	vertices[verticesOffset + 1] = quadVertices[1];
	vertices[verticesOffset + 2] = quadVertices[2];
	vertices[verticesOffset + 3] = quadVertices[3];

	const auto indicesOffset = cursor * 6;
	indices[indicesOffset + 0] = verticesOffset + 0;
	indices[indicesOffset + 1] = verticesOffset + 1;
	indices[indicesOffset + 2] = verticesOffset + 2;
	indices[indicesOffset + 3] = verticesOffset + 0;
	indices[indicesOffset + 4] = verticesOffset + 2;
	indices[indicesOffset + 5] = verticesOffset + 3;

	data[cursor++] = quad.getData();
}

void Batch::render(Shader &shader,
				   const std::shared_ptr<ID3D11DeviceContext> &context,
				   const std::shared_ptr<ID3D11RenderTargetView> &renderTargetView,
				   const D3D11_VIEWPORT &viewport) {
	if (cursor == 0) {
		return;
	}

	// Vertex Buffer
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	auto *vertexBufferPtr = vertexBuffer.get();
	context->IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
	// update vertex buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	auto res = context->Map(vertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(res)) {
		printf("Failed to map vertex buffer (%#08x)", (unsigned int)res);
		exit(1);
	}
	memcpy(mappedResource.pData, vertices.data(), sizeof(Vertex) * cursor * 4);
	context->Unmap(vertexBuffer.get(), 0);


	// Index Buffer
	context->IASetIndexBuffer(indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
	// update index buffer
	context->Map(indexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, indices.data(), sizeof(UINT) * cursor * 6);
	context->Unmap(indexBuffer.get(), 0);

	// Structured Buffer
	auto *structuredBufferViewPtr = structuredBufferView.get();
	context->VSSetShaderResources(0, 1, &structuredBufferViewPtr);
	// update structured buffer
	context->Map(structuredBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, data.data(), sizeof(VertexData) * cursor);
	context->Unmap(structuredBuffer.get(), 0);

	// Topology
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render target and viewport
	auto *renderTargetViewPtr = renderTargetView.get();
	context->OMSetRenderTargets(1, &renderTargetViewPtr, nullptr);
	context->RSSetViewports(1, &viewport);

	// Textures
	// TODO: store them directly as pointers
	std::vector<ID3D11ShaderResourceView *> texturesToSet{};
	for (auto i = textures.begin(); i != textures.end(); i++) {
		texturesToSet.push_back((*i).get());
	}
	context->PSSetShaderResources(0, texturesToSet.size(), texturesToSet.data());

	// Sampler
	auto *samplerStatePtr = samplerState.get();
	context->PSSetSamplers(0, 1, &samplerStatePtr);

	context->DrawIndexed(cursor * 6, 0, 0);

	cursor = 0;
	indices.fill(0);
	textures.clear();
}