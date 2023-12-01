#pragma once

#include "d3d11.h"
#include "memory"
#include <string_view>
#include <vector>

namespace squi {
	class Shader {
		std::shared_ptr<ID3D11VertexShader> vertexShader;
		std::shared_ptr<ID3D11PixelShader> pixelShader;
		std::shared_ptr<ID3D11InputLayout> inputLayout;

	public:
		Shader(std::string_view vertexShaderSource,
			   std::string_view fragmentShaderSource,
			   const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc,
			   const std::shared_ptr<ID3D11Device> &device);

		void use(std::shared_ptr<ID3D11DeviceContext> &context);
	};
}// namespace squi