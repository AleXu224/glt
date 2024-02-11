#include "engine.hpp"
#include "chrono"
#include "frame.hpp"
#include "iostream"
#include "stdexcept"
#include "vulkanIncludes.hpp"
#include <print>
#include <vector>


Engine::Vulkan::Vulkan() {}

Engine::Frame &Engine::Vulkan::getCurrentFrame() {
	return instance.frames.at(frameNumber % FrameBuffer);
}

void Engine::Vulkan::recreateSwapChain() {
	instance.recreateSwapChain();
}

void Engine::Vulkan::run(std::function<bool()> preDraw, std::function<void()> drawFunc) {
	try {
		auto &device = instance.device;

		uint32_t frames = 0;
		auto lastFrameTime = std::chrono::steady_clock::now();
		while (!glfwWindowShouldClose(instance.window.ptr)) {
			auto currentTime = std::chrono::steady_clock::now();
			using namespace std::chrono_literals;
			if (lastFrameTime + 1s <= currentTime) {
				glfwSetWindowTitle(instance.window.ptr, std::format("{} fps", frames).c_str());
				frames = 0;
				lastFrameTime = currentTime;
			}
			frames++;

			auto newFrameStartTime = std::chrono::steady_clock::now();
			deltaTime = newFrameStartTime - frameStartTime;
			frameStartTime = newFrameStartTime;

			instance.currentFrame = getCurrentFrame();

			if (!preDraw()) continue;

			auto resFence = device.waitForFences(*instance.currentFrame.get().renderFence, 1, 1000000000);
			if (resFence != vk::Result::eSuccess) throw std::runtime_error("Timeout waiting for render fence");

			auto [resNextImage, swapchainImageIndex] = (*device).acquireNextImageKHR(*instance.swapChain, 1000000000, *instance.currentFrame.get().swapchainSemaphore);
			if (resNextImage == vk::Result::eErrorOutOfDateKHR) {
				recreateSwapChain();
				continue;
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

			// cmd.setScissor(0, scissor);
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


			// if (resized) {
			// 	resized = false;
			// 	cmd.reset();
			// 	recreateSwapChain();
			// 	instance.frameEnd();
			// 	instance.currentFrame.get().endedEarly = true;
			// 	// device.signalSemaphore(vk::SemaphoreSignalInfo{
			// 	// 	.semaphore = *instance.currentFrame.get().swapchainSemaphore,
			// 	// });
			// 	continue;
			// }

			vk::SubmitInfo submitInfo{
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*instance.currentFrame.get().swapchainSemaphore,
				.pWaitDstStageMask = &waitStages,
				.commandBufferCount = 1,
				.pCommandBuffers = &*cmd,
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = &*instance.currentFrame.get().renderSemaphore,
			};

			instance.graphicsQueue.submit(submitInfo, *instance.currentFrame.get().renderFence);

			vk::PresentInfoKHR presentInfo{
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*instance.currentFrame.get().renderSemaphore,
				.swapchainCount = 1,
				.pSwapchains = &*instance.swapChain,
				.pImageIndices = &swapchainImageIndex,
			};

			try {
				auto res2 = instance.graphicsQueue.presentKHR(presentInfo);
				if (res2 == vk::Result::eErrorOutOfDateKHR || res2 == vk::Result::eSuboptimalKHR) {
					recreateSwapChain();
					instance.frameEnd();
					continue;
				} else if (res2 != vk::Result::eSuccess) {
					throw std::runtime_error("Failed to present\n");
				}
			} catch (const std::exception &e) {
				std::cerr << e.what() << std::endl;
				recreateSwapChain();
				instance.frameEnd();
				continue;
			}
			instance.frameEnd();
			frameNumber++;
		}
		device.waitIdle();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return;
	}
}
