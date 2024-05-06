#include "engine.hpp"
#include "chrono"
#include "frame.hpp"
#include "instance.hpp"
#include "iostream"
#include "stdexcept"
#include "vulkanIncludes.hpp"
#include <print>
#include <vector>
#include "GLFW/glfw3.h"


Engine::Vulkan::Vulkan() = default;

Engine::Frame &Engine::Vulkan::getCurrentFrame() {
	return instance.frames.at(frameNumber % FrameBuffer);
}

void Engine::Vulkan::recreateSwapChain() {
	instance.recreateSwapChain();
}

void Engine::Vulkan::run(const std::function<bool()> &preDraw, const std::function<void()> &drawFunc, const std::function<void()> &cleanupFunc) {
	this->preDraw = preDraw;
	this->drawFunc = drawFunc;
	try {
		while (!glfwWindowShouldClose(instance.window.ptr)) {
			draw();
		}
		instance.device.waitIdle();
		cleanupFunc();
	} catch (const std::exception &e) {
		std::println("Error: {}", e.what());
		
		instance.device.waitIdle();
		cleanupFunc();
		return;
	}
}

void Engine::Vulkan::draw() {
	auto &device = instance.device;
	auto newFrameStartTime = std::chrono::steady_clock::now();
	deltaTime = newFrameStartTime - frameStartTime;
	frameStartTime = newFrameStartTime;

	instance.currentFrame = getCurrentFrame();

	if (!preDraw()) return;

	auto resFence = device.waitForFences(*instance.currentFrame.get().renderFence, 1, 1000000000);
	if (resFence != vk::Result::eSuccess) throw std::runtime_error("Timeout waiting for render fence");

	auto [resNextImage, swapchainImageIndex] = instance.swapChain.acquireNextImage(1000000000, *instance.currentFrame.get().swapchainSemaphore);
	if (resNextImage == vk::Result::eErrorOutOfDateKHR) {
		recreateSwapChain();
		return;
	}

	device.resetFences(*instance.currentFrame.get().renderFence);

	auto &cmd = instance.currentFrame.get().commandBuffer;
	cmd.reset();
	cmd.begin({});

	using namespace std::chrono_literals;

	vk::ClearValue clearColor{.color = {.float32{{32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 1.0f}}}};

	vk::RenderPassBeginInfo renderPassInfo = {
		.renderPass = *instance.renderPass,
		.framebuffer = *instance.swapChainFramebuffers.at(swapchainImageIndex),
		.renderArea{
			.offset = {0, 0},
			.extent = instance.swapChainExtent,
		},
		.clearValueCount = 1,
		.pClearValues = &clearColor,
	};
	cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	vk::Viewport viewport{
		.x = 0.f,
		.y = 0.f,
		.width = static_cast<float>(instance.swapChainExtent.width),
		.height = static_cast<float>(instance.swapChainExtent.height),
		.minDepth = 0.f,
		.maxDepth = 1.f,
	};

	cmd.setViewport(0, viewport);

	instance.frameBegin();

	instance.pushScissor(squi::Rect::fromPosSize({0, 0}, {static_cast<float>(instance.swapChainExtent.width), static_cast<float>(instance.swapChainExtent.height)}));

	drawFunc();


	if (instance.currentPipelineFlush) (*instance.currentPipelineFlush)();
	instance.currentPipeline = nullptr;
	instance.currentPipelineFlush = nullptr;
	instance.popScissor();
	if (!instance.scissorStack.empty()) throw std::runtime_error("Scissor stack is not empty by the end of the frame!");

	cmd.endRenderPass();

	cmd.end();

	vk::Flags<vk::PipelineStageFlagBits> waitStages{
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
	};

	vk::SubmitInfo submitInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*instance.currentFrame.get().swapchainSemaphore,
		.pWaitDstStageMask = &waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &*cmd,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*instance.currentFrame.get().renderSemaphore,
	};

	graphicsQueueMutex.lock();
	instance.graphicsQueue.submit(submitInfo, *instance.currentFrame.get().renderFence);
	graphicsQueueMutex.unlock();

	vk::PresentInfoKHR presentInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*instance.currentFrame.get().renderSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &*instance.swapChain,
		.pImageIndices = &swapchainImageIndex,
	};

	try {
		graphicsQueueMutex.lock();
		auto res2 = instance.graphicsQueue.presentKHR(presentInfo);
		graphicsQueueMutex.unlock();
		if (res2 == vk::Result::eErrorOutOfDateKHR || res2 == vk::Result::eSuboptimalKHR) {
			recreateSwapChain();
			instance.frameEnd();
			return;
		}
		if (res2 != vk::Result::eSuccess) throw std::runtime_error("Failed to present\n");

	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		recreateSwapChain();
		instance.frameEnd();
		return;
	}
	instance.frameEnd();
	frameNumber++;
}
