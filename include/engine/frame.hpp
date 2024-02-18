#pragma once

#include "vulkanIncludes.hpp"
#include <print>
#include <vulkan/vulkan_raii.hpp>

namespace Engine {
	struct Frame {
		const size_t index;
		vk::raii::CommandPool commandPool = nullptr;
		vk::raii::CommandBuffer commandBuffer = nullptr;

		vk::raii::Fence renderFence = nullptr;
		vk::raii::Semaphore swapchainSemaphore = nullptr;
		vk::raii::Semaphore renderSemaphore = nullptr;

		Frame(size_t index, vk::raii::Device &device, uint32_t graphicsFamily);

		void recreateCommandBuffer(vk::raii::Device &device);

		void transitionSwapchainImage(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout) const;
	};
}// namespace Engine