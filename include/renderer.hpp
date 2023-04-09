#ifndef SQUI_RENDERER_HPP
#define SQUI_RENDERER_HPP

#include "array"
#include "batch.hpp"
#include "list"
#include "shader.hpp"
#include "vertex.hpp"
#include "memory"
#include "chrono"

namespace squi {
	class Renderer {
		static std::unique_ptr<Renderer> instance;
		std::unique_ptr<Batch> batch{};
		std::unique_ptr<Shader> shader{};
		DirectX::XMFLOAT4X4 projectionMatrix{};
		std::shared_ptr<ID3D11Buffer> projectionMatrixBuffer{};
		std::chrono::duration<double> deltaTime = std::chrono::duration<double>::zero();
		std::chrono::time_point<std::chrono::steady_clock> currentFrameTime = std::chrono::steady_clock::now();

		D3D11_VIEWPORT viewport{};
		std::shared_ptr<ID3D11Device> device{};
		std::shared_ptr<ID3D11DeviceContext> deviceContext{};
		std::shared_ptr<IDXGISwapChain> swapChain{};
		std::shared_ptr<ID3D11RenderTargetView> renderTargetView{};
		std::shared_ptr<ID3D11Resource> backBuffer{};
		std::shared_ptr<ID3D11BlendState> blendState{};
		std::vector<Rect> clipRects{};

	public:
		static Renderer &getInstance();

		void addClipRect(const Rect &rect);
		const Rect &getCurrentClipRect() const;
		void popClipRect();

		void addQuad(Quad &quad);

		void prepare();

		void render();

		void updateScreenSize(int width, int height);
		void updateDeltaTime(std::chrono::duration<double> time);
		void updateCurrentFrameTime(std::chrono::time_point<std::chrono::steady_clock> time);

		[[nodiscard]] std::chrono::duration<double> getDeltaTime() const;
		[[nodiscard]] std::chrono::time_point<std::chrono::steady_clock> getCurrentFrameTime() const;

		[[nodiscard]] const D3D11_VIEWPORT &getViewport() const;
		[[nodiscard]] std::shared_ptr<ID3D11Device> getDevice() const;
		[[nodiscard]] std::shared_ptr<ID3D11DeviceContext> getDeviceContext() const;
		[[nodiscard]] std::shared_ptr<IDXGISwapChain> getSwapChain() const;
		[[nodiscard]] std::shared_ptr<ID3D11RenderTargetView> getRenderTargetView() const;

		Renderer(HWND hwnd, int width, int height);
		static void initialize(HWND hwnd, int width, int height);
	};
}// namespace squi

#endif