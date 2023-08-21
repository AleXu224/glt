#include "shader.hpp"
#include <d3dcompiler.h>
#include <print>

using namespace squi;

Shader::Shader(const char *vertexShaderSource,
			   const char *fragmentShaderSource,
			   const std::shared_ptr<ID3D11Device> &device) {
	ID3DBlob *vertexShaderBlob = nullptr;
	ID3DBlob *fragmentShaderBlob = nullptr;
	ID3DBlob *errorBlob = nullptr;

	// Vertex Shader
	auto res = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, &errorBlob);
	if (res != S_OK) {
		std::print("Failed to compile vertex shader ({:#08x}):\n{}", (unsigned int) res, (char *) errorBlob->GetBufferPointer());
		exit(1);
	}

	ID3D11VertexShader *vertexShaderPtr = nullptr;
	res = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &vertexShaderPtr);
	if (res != S_OK) {
		std::println("Failed to create vertex shader ({:#08x})", (unsigned int) res);
		exit(1);
	}
	vertexShader.reset(vertexShaderPtr, [](ID3D11VertexShader *shaderPtr) {
		shaderPtr->Release();
	});

	// Pixel shader
	res = D3DCompile(fragmentShaderSource, strlen(fragmentShaderSource), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &fragmentShaderBlob, &errorBlob);
	if (res != S_OK) {
		std::print("Failed to compile pixel shader ({:#08x}):\n{}", (unsigned int) res, (char *) errorBlob->GetBufferPointer());
		exit(1);
	}

	ID3D11PixelShader *pixelShaderPtr = nullptr;
	res = device->CreatePixelShader(fragmentShaderBlob->GetBufferPointer(), fragmentShaderBlob->GetBufferSize(), nullptr, &pixelShaderPtr);
	if (res != S_OK) {
		std::println("Failed to create pixel shader ({:#08x})", (unsigned int) res);
		exit(1);
	}
	pixelShader.reset(pixelShaderPtr, [](ID3D11PixelShader *shaderPtr) {
		shaderPtr->Release();
	});

	// Input layout
	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
		{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXUV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"ID", 0, DXGI_FORMAT_R32_UINT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}};

	ID3D11InputLayout *inputLayoutPtr = nullptr;
	res = device->CreateInputLayout(inputLayoutDesc, 3, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayoutPtr);
	if (res != S_OK) {
		std::println("Failed to create input layout ({:#08x})", (unsigned int) res);
		exit(1);
	}
	inputLayout.reset(inputLayoutPtr, [](ID3D11InputLayout *inputLayoutPtr) {
		inputLayoutPtr->Release();
	});

	vertexShaderBlob->Release();
	fragmentShaderBlob->Release();
}

void Shader::use(std::shared_ptr<ID3D11DeviceContext> &context) {
	context->VSSetShader(vertexShader.get(), nullptr, 0);
	context->PSSetShader(pixelShader.get(), nullptr, 0);
	context->IASetInputLayout(inputLayout.get());
}