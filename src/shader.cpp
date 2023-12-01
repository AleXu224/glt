#include "shader.hpp"
#include <d3dcompiler.h>
#include <print>
#include <string_view>

using namespace squi;

Shader::Shader(std::string_view vertexShaderSource,
			   std::string_view fragmentShaderSource,
			   const std::vector<D3D11_INPUT_ELEMENT_DESC> &inputLayoutDesc,
			   const std::shared_ptr<ID3D11Device> &device) {
	ID3DBlob *vertexShaderBlob = nullptr;
	ID3DBlob *fragmentShaderBlob = nullptr;
	ID3DBlob *errorBlob = nullptr;

	// Vertex Shader
	auto res = D3DCompile(vertexShaderSource.data(), vertexShaderSource.size(), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, &errorBlob);
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
	res = D3DCompile(fragmentShaderSource.data(), fragmentShaderSource.size(), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &fragmentShaderBlob, &errorBlob);
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
	ID3D11InputLayout *inputLayoutPtr = nullptr;
	res = device->CreateInputLayout(inputLayoutDesc.data(), inputLayoutDesc.size(), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayoutPtr);
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