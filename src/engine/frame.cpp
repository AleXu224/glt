#include "frame.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

using namespace Engine;
Engine::Frame::Frame(size_t index, vk::raii::Device &device, uint32_t graphicsFamily) : index(index) {
	vk::CommandPoolCreateInfo poolInfo{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = graphicsFamily,
	};

	commandPool = device.createCommandPool(poolInfo);

	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = *commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1,
	};

	commandBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());

	renderFence = {device, {.flags = vk::FenceCreateFlagBits::eSignaled}};
	swapchainSemaphore = {device, vk::SemaphoreCreateInfo{}};
	renderSemaphore = {device, vk::SemaphoreCreateInfo{}};
}

void Engine::Frame::recreateCommandBuffer(vk::raii::Device &device) {
	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = *commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1,
	};

	commandBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());
}

void Engine::Frame::transitionSwapchainImage(vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout) {
	vk::ImageAspectFlags aspectMask = (newLayout == vk::ImageLayout::eDepthAttachmentOptimal) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;

	vk::ImageSubresourceRange subrange{
		.aspectMask = aspectMask,
		.baseMipLevel = 0,
		.levelCount = VK_REMAINING_MIP_LEVELS,
		.baseArrayLayer = 0,
		.layerCount = VK_REMAINING_ARRAY_LAYERS,
	};

	vk::ImageMemoryBarrier2 imageBarrier{
		.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
		.srcAccessMask = vk::AccessFlagBits2::eMemoryWrite,
		.dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
		.dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead,
		.oldLayout = currentLayout,
		.newLayout = newLayout,
		.image = image,
		.subresourceRange = subrange,
	};

	vk::DependencyInfo depInfo{
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &imageBarrier,
	};
	commandBuffer.pipelineBarrier2(depInfo);
}
