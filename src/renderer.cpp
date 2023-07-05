#include "renderer.hpp"
#include <d3d11.h>

using namespace squi;

auto vertexShaderHlsl = R"(
		struct VertexData {
			float4 color;
			float4 borderColor;
			float2 pos;
			float2 size;
			float2 offset;
			float borderRadius;
			float borderSize;
			uint textureId;
			uint textureType;
			float4 clipRect;
			float clipBorderRadius;
		};

		StructuredBuffer<VertexData> data : register(t0);

		cbuffer ProjectionMatrix : register(b0) {
			float4x4 uProjectionMatrix;
		};

		struct VS_OUTPUT {
			float4 color : COLOR;
			float2 uv : TEXCOORD0;
			float2 texUV : TEXCOORD1;
			float2 size : TEXCOORD2;
			float borderRadius : TEXCOORD3;
			float borderSize : TEXCOORD4;
			float4 borderColor : TEXCOORD5;
			uint textureId : TEXCOORD6;
			uint textureType : TEXCOORD7;
			float4 pos : SV_POSITION;
			float2 clipSize : TEXCOORD8;
			float2 clipUV : TEXCOORD9;
			float clipBorderRadius : TEXCOORD10;
		};

		VS_OUTPUT main(float2 aUV : UV, float2 aTexUV : TEXUV, uint aID : ID) {
			VertexData quadData = data[aID];
			VS_OUTPUT output;
			output.color = quadData.color;
			output.uv = aUV;
			output.texUV = aTexUV;
			output.size = quadData.size;
			output.borderRadius = quadData.borderRadius;
			output.borderSize = quadData.borderSize;
			output.borderColor = quadData.borderColor;
			output.textureId = quadData.textureId;
			output.textureType = quadData.textureType;
			float2 pos = quadData.offset + quadData.pos + (aUV * quadData.size);
			output.pos = mul(uProjectionMatrix, float4(pos, 0.0, 1.0));
//			output.pos = float4(pos, 0.0, 1.0);
			output.clipSize = quadData.clipRect.zw - quadData.clipRect.xy;
			output.clipUV = (pos - quadData.clipRect.xy) / output.clipSize;
			output.clipBorderRadius = quadData.clipBorderRadius;
			return output;
		}
	)";
// TODO: Borders do not work with border radius = 0
auto fragmentShaderHlsl = R"(
		struct PS_INPUT {
			float4 color : COLOR;
			float2 uv : TEXCOORD0;
			float2 texUV : TEXCOORD1;
			float2 size : TEXCOORD2;
			float borderRadius : TEXCOORD3;
			float borderSize : TEXCOORD4;
			float4 borderColor : TEXCOORD5;
			uint textureId : TEXCOORD6;
			uint textureType : TEXCOORD7;
			float4 pos : SV_POSITION;
			float2 clipSize : TEXCOORD8;
			float2 clipUV : TEXCOORD9;
			float clipBorderRadius : TEXCOORD10;
		};

		Texture2D uTexture[16] : register(t0);
		SamplerState samp1 : register(s0);

		float4 textureByIndex(uint index, float2 uv) {
			[call] switch (index) {
				case 0: return uTexture[0].Sample(samp1, uv);
				case 1: return uTexture[1].Sample(samp1, uv);
				case 2: return uTexture[2].Sample(samp1, uv);
				case 3: return uTexture[3].Sample(samp1, uv);
				case 4: return uTexture[4].Sample(samp1, uv);
				case 5: return uTexture[5].Sample(samp1, uv);
				case 6: return uTexture[6].Sample(samp1, uv);
				case 7: return uTexture[7].Sample(samp1, uv);
				case 8: return uTexture[8].Sample(samp1, uv);
				case 9: return uTexture[9].Sample(samp1, uv);
				case 10: return uTexture[10].Sample(samp1, uv);
				case 11: return uTexture[11].Sample(samp1, uv);
				case 12: return uTexture[12].Sample(samp1, uv);
				case 13: return uTexture[13].Sample(samp1, uv);
				case 14: return uTexture[14].Sample(samp1, uv);
				case 15: return uTexture[15].Sample(samp1, uv);
				default: return 0.0;
			}
		}

		float udRoundBox(float2 p, float2 b, float r) {
			float2 q = abs(p) - b + r;
			return min(max(q.x,q.y),0.0) + length(max(q, 0.0)) - r;
		}

		float4 main(PS_INPUT input) : SV_TARGET {
			if (input.clipUV.x < 0.0 || input.clipUV.x > 1.0 || input.clipUV.y < 0.0 || input.clipUV.y > 1.0) {
				discard;
			}
			float c = udRoundBox((input.clipUV * input.clipSize - 0.5) - (0.5 * input.clipSize - 0.5), (0.5 * input.clipSize - 0.5), input.clipBorderRadius);
			float2 coords = (input.uv * input.size) - 0.5;
			float2 halfRes = (0.5 * input.size) - 0.5;
			float borderRadius = min(input.borderRadius, min(halfRes.x, halfRes.y));
			float b = udRoundBox(coords - halfRes, halfRes, borderRadius);
			float4 outColor = input.color;
			outColor.xyz *= input.color.a;
			float bSize = input.borderSize;
			if (b > -bSize && bSize > 0.0) {
				float4 borderColor = input.borderColor;
				borderColor.xyz *= input.borderColor.a;
				outColor = lerp(outColor, borderColor, smoothstep(0.0, 1.0, b + bSize));
			}
			if (input.textureType == 1) {
				float4 texColor = textureByIndex(input.textureId, input.texUV);
				float3 color = texColor.rgb * texColor.a;
				return float4(color, texColor.a);
			}
			if (input.textureType == 2) {
				float alpha = textureByIndex(input.textureId, input.texUV).r * input.color.a;
				float3 color = input.color.rgb * alpha;
				return float4(color.r, color.g, color.b, alpha);
			}
			float4 retColor = lerp(outColor, float4(0.0, 0.0, 0.0, 0.0), smoothstep(0.0, 1.0, b));
			return lerp(retColor, float4(0.0, 0.0, 0.0, 0.0), smoothstep(0.0, 1.0, c));
		}
	)";

void Renderer::addClipRect(const Rect &clipRect, float clipBorderRadius) {
	if (!clipRects.empty())
		clipRects.push_back({clipRects.back().rect.overlap(clipRect), clipBorderRadius});
	else
		clipRects.push_back({clipRect, clipBorderRadius});
}

const Renderer::ClipRect &Renderer::getCurrentClipRect() const {
	return clipRects.back();
}

void Renderer::popClipRect() {
	if (clipRects.empty()) {
		throw std::runtime_error("Clip rects empty, can't pop");
		exit(1);
	}
	clipRects.pop_back();
}

void Renderer::addQuad(Quad &quad) {
	quad.setClipRect(clipRects.back().rect, clipRects.back().borderRadius);
	batch->addQuad(quad, *shader, deviceContext, renderTargetView, viewport);
}

void Renderer::prepare() {
	if (!clipRects.empty()) {
		throw std::runtime_error("Clip rects not empty at end of frame, you forgot to pop one or more clip rects");
		exit(1);
	}
	addClipRect(Rect(vec2(0, 0), vec2(viewport.Width, viewport.Height)));
	shader->use(deviceContext);
	auto *projectionMatrixBufferPtr = projectionMatrixBuffer.get();
	deviceContext->VSSetConstantBuffers(0, 1, &projectionMatrixBufferPtr);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(projectionMatrixBufferPtr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &projectionMatrix, sizeof(DirectX::XMFLOAT4X4));
	deviceContext->Unmap(projectionMatrixBufferPtr, 0);
}

void Renderer::render() {
	batch->render(*shader, deviceContext, renderTargetView, viewport);
}

void Renderer::updateScreenSize(int width, int height) {
	projectionMatrix._11 = 2.0f / (float) width;
	projectionMatrix._22 = -2.0f / (float) height;

	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	renderTargetView.reset();
	backBuffer.reset();

	swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

	ID3D11Texture2D *backBufferPtr = nullptr;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *) &backBufferPtr);
	backBuffer.reset(backBufferPtr, [](ID3D11Texture2D *backBufferPtr) { backBufferPtr->Release(); });

	ID3D11RenderTargetView *renderTargetViewPtr = nullptr;
	device->CreateRenderTargetView(backBuffer.get(), nullptr, &renderTargetViewPtr);
	renderTargetView.reset(renderTargetViewPtr, [](ID3D11RenderTargetView *renderTargetViewPtr) { renderTargetViewPtr->Release(); });

	deviceContext->OMSetRenderTargets(1, &renderTargetViewPtr, nullptr);

	viewport.Width = (float) width;
	viewport.Height = (float) height;
	deviceContext->RSSetViewports(1, &viewport);
}

void Renderer::updateDeltaTime(std::chrono::duration<double> time) {
	this->deltaTime = time;
}

void Renderer::updatePollTime(std::chrono::duration<double> time) {
	this->pollTime = time;
}

void Renderer::updateUpdateTime(std::chrono::duration<double> time) {
	this->updateTime = time;
}

void Renderer::updateDrawTime(std::chrono::duration<double> time) {
	this->drawTime = time;
}

void Renderer::updatePresentTime(std::chrono::duration<double> time) {
	this->presentTime = time;
}

void Renderer::updateCurrentFrameTime(std::chrono::time_point<std::chrono::steady_clock> time) {
	this->currentFrameTime = time;
}

std::chrono::duration<double> Renderer::getDeltaTime() const {
	return this->deltaTime;
}

std::chrono::duration<double> Renderer::getPollTime() const {
	return this->pollTime;
}

std::chrono::duration<double> Renderer::getUpdateTime() const {
	return this->updateTime;
}

std::chrono::duration<double> Renderer::getDrawTime() const {
	return this->drawTime;
}

std::chrono::duration<double> Renderer::getPresentTime() const {
	return this->presentTime;
}

std::chrono::time_point<std::chrono::steady_clock> Renderer::getCurrentFrameTime() const {
	return this->currentFrameTime;
}

void Renderer::initialize(HWND hwnd, int width, int height) {
	// Viewport
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// Swap Chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 3;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	ID3D11Device *devicePtr = nullptr;
	ID3D11DeviceContext *contextPtr = nullptr;
	IDXGISwapChain *swapChainPtr = nullptr;
	ID3D11RenderTargetView *renderTargetViewPtr = nullptr;
	ID3D11Resource *backBufferPtr = nullptr;
	auto res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChainPtr,
		&devicePtr,
		nullptr,
		&contextPtr);

	if (res != S_OK) {
		res = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&swapChainPtr,
			&devicePtr,
			nullptr,
			&contextPtr);
		if (res != S_OK) {
			printf("Failed to create device and swap chain (%#08x)\n", (unsigned int) res);
			exit(1);
		}
		printf("Hardware acceleration not available, using WARP\n");
	}
	swapChainPtr->GetBuffer(0, __uuidof(ID3D11Resource), (void **) &backBufferPtr);
	devicePtr->CreateRenderTargetView(backBufferPtr, nullptr, &renderTargetViewPtr);

	device.reset(devicePtr, [](ID3D11Device *devicePtr) { devicePtr->Release(); });
	deviceContext.reset(contextPtr, [](ID3D11DeviceContext *context) { context->Release(); });
	swapChain.reset(swapChainPtr, [](IDXGISwapChain *swapChainPtr) { swapChainPtr->Release(); });
	renderTargetView.reset(renderTargetViewPtr, [](ID3D11RenderTargetView *renderTargetViewPtr) { renderTargetViewPtr->Release(); });
	backBuffer.reset(backBufferPtr, [](ID3D11Resource *backBufferPtr) { backBufferPtr->Release(); });

	// Enable alpha blending
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	ID3D11BlendState *blendStatePtr = nullptr;
	device->CreateBlendState(&blendDesc, &blendStatePtr);
	blendState.reset(blendStatePtr, [](ID3D11BlendState *blendStatePtr) { blendStatePtr->Release(); });
	deviceContext->OMSetBlendState(blendState.get(), nullptr, 0xffffffff);

	shader = std::make_unique<Shader>(vertexShaderHlsl, fragmentShaderHlsl, device);
	batch = std::make_unique<Batch>(device);

	// projection matrix to identity
	DirectX::XMStoreFloat4x4(&this->projectionMatrix, DirectX::XMMatrixIdentity());
	// Scale to 800x600 and flip y axis
	projectionMatrix._11 = 2.0f / static_cast<float>(width);
	projectionMatrix._22 = -2.0f / static_cast<float>(height);
	// Translate to topleft corner
	projectionMatrix._41 = -1.0f;
	projectionMatrix._42 = 1.0f;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = &projectionMatrix;

	ID3D11Buffer *projectionMatrixBufferPtr;
	device->CreateBuffer(&bufferDesc, &subresourceData, &projectionMatrixBufferPtr);
	projectionMatrixBuffer.reset(projectionMatrixBufferPtr, [](ID3D11Buffer *buffer) { buffer->Release(); });
}

const D3D11_VIEWPORT &Renderer::getViewport() const {
	return viewport;
}

std::shared_ptr<ID3D11Device> Renderer::getDevice() const {
	return device;
}

std::shared_ptr<ID3D11DeviceContext> Renderer::getDeviceContext() const {
	return deviceContext;
}

std::shared_ptr<IDXGISwapChain> Renderer::getSwapChain() const {
	return swapChain;
}

std::shared_ptr<ID3D11RenderTargetView> Renderer::getRenderTargetView() const {
	return renderTargetView;
}