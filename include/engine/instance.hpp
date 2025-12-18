#pragma once

#include "frame.hpp"
#include "observer.hpp"
#include "rect.hpp"
#include "vulkanIncludes.hpp"
#include "window.hpp"
#include <functional>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>


namespace Engine {
	struct Instance {
		Window window;
		vk::raii::SurfaceKHR surface;

		vk::Extent2D swapChainExtent;
		vk::raii::SwapchainKHR swapChain;
		vk::Format swapChainImageFormat;

		std::vector<vk::Image> swapChainImages;
		std::vector<vk::raii::ImageView> swapChainImageViews;
		vk::raii::RenderPass renderPass;
		std::vector<vk::raii::Framebuffer> swapChainFramebuffers;

		std::vector<std::function<void()>> nextFrameTasks{};

		std::vector<Frame> frames;
		std::reference_wrapper<Frame> currentFrame;

		void *currentPipeline = nullptr;
		std::function<void()> *currentPipelineFlush = nullptr;

		squi::VoidObservable frameEndEvent{};
		squi::VoidObservable frameBeginEvent{};

		struct ScissorEntry {
			// A scissor entry that assumes there are no transforms, should be used for bounds checking
			squi::Rect logical;
			// This represents the actual scissor that is being used to render
			squi::Rect physical;
		};

		std::vector<ScissorEntry> scissorStack{};
		void pushScissor(const squi::Rect &rect) {
			if (currentPipelineFlush) (*currentPipelineFlush)();
			auto transformedRect = rect.transformed(getTransform());
			if (!scissorStack.empty()) {
				scissorStack.push_back(ScissorEntry{
					.logical = rect.overlap(scissorStack.back().logical),
					.physical = transformedRect.overlap(scissorStack.back().physical),
				});
			} else {
				scissorStack.push_back(ScissorEntry{
					.logical = rect,
					.physical = transformedRect,
				});
			}
			const auto &r = scissorStack.back().physical;
			auto sz = r.size().rounded();
			auto pos = r.getTopLeft();
			currentFrame.get().commandBuffer.setScissor(
				0,
				vk::Rect2D{
					.offset{
						.x = static_cast<int32_t>(pos.x),
						.y = static_cast<int32_t>(pos.y),
					},
					.extent{
						.width = static_cast<uint32_t>(std::max(sz.x, 0.f)),
						.height = static_cast<uint32_t>(std::max(sz.y, 0.f)),
					},
				}
			);
		}
		void popScissor() {
			if (currentPipelineFlush) (*currentPipelineFlush)();
			scissorStack.pop_back();
			if (scissorStack.empty()) return;
			auto rect = scissorStack.back().physical;
			auto sz = rect.size().rounded();
			auto pos = rect.getTopLeft().rounded();
			currentFrame.get().commandBuffer.setScissor(
				0, vk::Rect2D{
					   .offset{
						   .x = static_cast<int32_t>(pos.x),
						   .y = static_cast<int32_t>(pos.y),
					   },
					   .extent{
						   .width = static_cast<uint32_t>(std::max(sz.x, 0.f)),
						   .height = static_cast<uint32_t>(std::max(sz.y, 0.f)),
					   },
				   }
			);
		}

		static inline uint32_t transformIndex = 0;
		std::vector<std::pair<uint32_t, glm::mat4>> transformStack{};
		void pushTransform(glm::mat4 matrix) {
			if (currentPipelineFlush) (*currentPipelineFlush)();
			if (!transformStack.empty()) {
				matrix = transformStack.back().second * matrix;
			}
			transformStack.push_back({++transformIndex, matrix});
		}
		void popTransform() {
			if (currentPipelineFlush) (*currentPipelineFlush)();
			transformStack.pop_back();
		}
		[[nodiscard]] uint32_t getTransformIndex() const {
			if (transformStack.empty()) return 0;
			return transformStack.back().first;
		}
		[[nodiscard]] glm::mat4 getTransform() const {
			if (transformStack.empty()) return glm::mat4(1.f);
			return transformStack.back().second;
		}

		Instance(WindowOptions windowOptions = {});

		void recreateSwapChain();

		void frameEnd() const {
			frameEndEvent.notify();
		}

		void frameBegin() const {
			frameBeginEvent.notify();
		}

	private:
		struct SwapChainSupportDetails {
			vk::SurfaceCapabilitiesKHR capabilities;
			std::vector<vk::SurfaceFormatKHR> formats;
			std::vector<vk::PresentModeKHR> presentModes;
		};

		[[nodiscard]] vk::raii::SurfaceKHR createSurface() const;

		[[nodiscard]] vk::raii::SwapchainKHR createSwapChain(bool recreating);
		[[nodiscard]] vk::raii::SwapchainKHR createSwapChain(bool recreating, const SwapChainSupportDetails &swapChainSupport);
		[[nodiscard]] std::vector<vk::Image> createSwapChainImages() const;
		[[nodiscard]] vk::Format createSwapChainImageFormat();
		[[nodiscard]] vk::Extent2D createExtent();
		[[nodiscard]] std::vector<vk::raii::ImageView> createImageViews();

		[[nodiscard]] vk::raii::RenderPass createRenderPass();
		[[nodiscard]] std::vector<vk::raii::Framebuffer> createFramebuffers();

		[[nodiscard]] bool checkValidationLayers() const;

		[[nodiscard]] Engine::Instance::SwapChainSupportDetails querySwapChainSupport(const vk::raii::PhysicalDevice &device) const;
		[[nodiscard]] static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
		[[nodiscard]] static vk::PresentModeKHR chooseSwapPresentMode();
		[[nodiscard]] vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const;
	};
}// namespace Engine