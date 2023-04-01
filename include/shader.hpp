#ifndef SQUI_SHADER_HPP
#define SQUI_SHADER_HPP

#include "d3d11.h"
#include "d3dcompiler.h"
#include "DirectXMath.h"
#include "vector"
#include "memory"

namespace squi {
	class Shader {
		std::shared_ptr<ID3D11VertexShader> vertexShader;
		std::shared_ptr<ID3D11PixelShader> pixelShader;
		std::shared_ptr<ID3D11InputLayout> inputLayout;

	public:
		Shader(const char *vertexShaderSource,
			   const char *fragmentShaderSource,
			   const std::shared_ptr<ID3D11Device> &device);

		void use(std::shared_ptr<ID3D11DeviceContext> &context);
	};
}// namespace squi

#endif