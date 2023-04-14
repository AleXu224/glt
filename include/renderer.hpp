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
		std::chrono::duration<double> pollTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> updateTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> drawTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> presentTime = std::chrono::duration<double>::zero();
		std::chrono::time_point<std::chrono::steady_clock> currentFrameTime = std::chrono::steady_clock::now();

		D3D11_VIEWPORT viewport{};
		std::shared_ptr<ID3D11Device> device{};
		std::shared_ptr<ID3D11DeviceContext> deviceContext{};
		std::shared_ptr<IDXGISwapChain> swapChain{};
		std::shared_ptr<ID3D11RenderTargetView> renderTargetView{};
		std::shared_ptr<ID3D11Resource> backBuffer{};
		std::shared_ptr<ID3D11BlendState> blendState{};
		struct ClipRect {
			Rect rect;
			float borderRadius;
		};
		std::vector<ClipRect> clipRects{};

	public:
		static Renderer &getInstance();

		void addClipRect(const Rect &rect, float clipBorderRadius = 0.0f);
		[[nodiscard]] const ClipRect &getCurrentClipRect() const;
		void popClipRect();

		void addQuad(Quad &quad);

		void prepare();

		void render();

		void updateScreenSize(int width, int height);
		void updateDeltaTime(std::chrono::duration<double> time);
		void updatePollTime(std::chrono::duration<double> time);
		void updateUpdateTime(std::chrono::duration<double> time);
		void updateDrawTime(std::chrono::duration<double> time);
		void updatePresentTime(std::chrono::duration<double> time);
		void updateCurrentFrameTime(std::chrono::time_point<std::chrono::steady_clock> time);

		[[nodiscard]] std::chrono::duration<double> getDeltaTime() const;
		[[nodiscard]] std::chrono::duration<double> getPollTime() const;
		[[nodiscard]] std::chrono::duration<double> getUpdateTime() const;
		[[nodiscard]] std::chrono::duration<double> getDrawTime() const;
		[[nodiscard]] std::chrono::duration<double> getPresentTime() const;
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