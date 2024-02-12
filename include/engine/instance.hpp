#pragma once

#include "any"
#include "frame.hpp"
#include "rect.hpp"
#include "vulkanIncludes.hpp"
#include "window.hpp"
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>


namespace Engine {
	struct Instance {
		Window window;

		vk::raii::Context context{};
		vk::raii::Instance instance;
		vk::raii::SurfaceKHR surface;
		vk::raii::PhysicalDevice physicalDevice;
		vk::raii::Device device;
		vk::raii::Queue graphicsQueue;
		vk::raii::Queue presentQueue;

		vk::raii::SwapchainKHR swapChain;
		vk::Format swapChainImageFormat;
		vk::Extent2D swapChainExtent;

		std::vector<vk::Image> swapChainImages;
		std::vector<vk::raii::ImageView> swapChainImageViews;
		vk::raii::RenderPass renderPass;
		std::vector<vk::raii::Framebuffer> swapChainFramebuffers;

		std::vector<Frame> frames;
		std::reference_wrapper<Frame> currentFrame;

		void *currentPipeline = nullptr;
		std::function<void()> *currentPipelineFlush = nullptr;

		std::vector<std::any> pipelines{};

		template<class T, class U>
		T &createPipeline(U &&args) {
			auto &val = pipelines.emplace_back(std::make_any<std::shared_ptr<T>>(std::make_shared<T>(std::forward<U>(args))));
			return *std::any_cast<std::shared_ptr<T> &>(val);
		}

		std::vector<squi::Rect> scissorStack{};
		void pushScissor(const squi::Rect &rect) {
			if (currentPipelineFlush) (*currentPipelineFlush)();
			if (!scissorStack.empty()) {
				scissorStack.push_back(rect.overlap(scissorStack.back()));
			} else {
				scissorStack.push_back(rect);
			}
			const auto &r = scissorStack.back();
			auto sz = r.size().rounded();
			auto pos = r.getTopLeft();
			currentFrame.get().commandBuffer.setScissor(
				0, vk::Rect2D{
					   .offset{
						   static_cast<int32_t>(pos.x),
						   static_cast<int32_t>(pos.y),
					   },
					   .extent{
						   static_cast<uint32_t>(std::max(sz.x, 0.f)),
						   static_cast<uint32_t>(std::max(sz.y, 0.f)),
					   },
				   }
			);
		}
		void popScissor() {
			if (currentPipelineFlush) (*currentPipelineFlush)();
			scissorStack.pop_back();
			if (scissorStack.empty()) return;
			auto rect = scissorStack.back();
			auto sz = rect.size().rounded();
			auto pos = rect.getTopLeft().rounded();
			currentFrame.get().commandBuffer.setScissor(
				0, vk::Rect2D{
					   .offset{
						   static_cast<int32_t>(pos.x),
						   static_cast<int32_t>(pos.y),
					   },
					   .extent{
						   static_cast<uint32_t>(sz.x),
						   static_cast<uint32_t>(sz.y),
					   },
				   }
			);
		}

		Instance();

		void recreateSwapChain();
		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			[[nodiscard]] bool isComplete() const {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};
		QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice &device);
		void frameEnd() {
			for (auto &listener: frameEndListeners) listener();
		}
		void listenFrameEnd(const std::function<void(void)> &callback) {
			frameEndListeners.push_back(callback);
		}

		void frameBegin() {
			for (auto &listener: frameBeginListeners) listener();
		}
		void listenFrameBegin(const std::function<void(void)> &callback) {
			frameBeginListeners.push_back(callback);
		}

	private:
		std::vector<std::function<void(void)>> frameEndListeners{};
		std::vector<std::function<void(void)>> frameBeginListeners{};

		[[nodiscard]] vk::raii::Instance createInstance();
		[[nodiscard]] vk::raii::SurfaceKHR createSurface();
		[[nodiscard]] vk::raii::PhysicalDevice selectPhysicalDevice();
		[[nodiscard]] vk::raii::Device createLogicalDevice();
		[[nodiscard]] vk::raii::Queue createGraphicsQueue();
		[[nodiscard]] vk::raii::Queue createPresentQueue();


		[[nodiscard]] vk::raii::SwapchainKHR createSwapChain(bool recreating);
		[[nodiscard]] std::vector<vk::Image> createSwapChainImages();
		[[nodiscard]] vk::Format createSwapChainImageFormat();
		[[nodiscard]] vk::Extent2D createExtent();
		[[nodiscard]] std::vector<vk::raii::ImageView> createImageViews();

		[[nodiscard]] vk::raii::RenderPass createRenderPass();
		[[nodiscard]] std::vector<vk::raii::Framebuffer> createFramebuffers();

		bool checkValidationLayers();


		struct SwapChainSupportDetails {
			vk::SurfaceCapabilitiesKHR capabilities;
			std::vector<vk::SurfaceFormatKHR> formats;
			std::vector<vk::PresentModeKHR> presentModes;
		};

		SwapChainSupportDetails querySwapChainSupport(const vk::raii::PhysicalDevice &device);
		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
		static vk::PresentModeKHR chooseSwapPresentMode();
		vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
	};
}// namespace Engine