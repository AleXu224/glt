#pragma once

#include "vulkan.hpp"
#include <any>

namespace Engine {
	struct CommandBufferContainer {
		vk::raii::CommandPool commandPool;
		vk::raii::CommandBuffer commandBuffer;
		std::vector<std::any> preservedResources{};

		CommandBufferContainer(std::pair<vk::raii::CommandPool, vk::raii::CommandBuffer> &&cmdPair)
			: commandPool(std::move(cmdPair.first)),
			  commandBuffer(std::move(cmdPair.second)) {}

		void pushResource(std::any resource) {
			preservedResources.emplace_back(std::move(resource));
		}
	};
	using BufferContainer = std::shared_ptr<CommandBufferContainer>;

	struct CommandQueue {
	private:
		static inline std::mutex mtx{};
		// Keeps Command Pools and Command Buffers alive until the end of the frame
		static inline std::vector<BufferContainer> storage{};

	public:
		static inline BufferContainer makeCommandBuffer() {
			auto cmdPair = Vulkan::makeCommandBuffer();
			return std::make_shared<CommandBufferContainer>(std::move(cmdPair));
		}


		static inline void push(const BufferContainer &cmdContainer) {
			std::scoped_lock lock{mtx};
			storage.emplace_back(cmdContainer);
		}

		[[nodiscard]] static inline std::scoped_lock<std::mutex> acquireLock() {
			return std::scoped_lock<std::mutex>{mtx};
		}

		static inline void cleanup() {
			std::scoped_lock lock{mtx};
			storage.clear();
		}

		static inline void frameEnd() {
			std::vector<BufferContainer> toSubmit;
			{
				std::scoped_lock lock{mtx};
				if (storage.empty()) return;
				toSubmit.swap(storage);
			}

			std::vector<vk::CommandBuffer> cmds;
			cmds.reserve(toSubmit.size());
			for (auto &cmdContainer: toSubmit) {
				cmds.push_back(*cmdContainer->commandBuffer);
			}

			vk::SubmitInfo submitInfo{
				.commandBufferCount = static_cast<uint32_t>(cmds.size()),
				.pCommandBuffers = cmds.data(),
			};

			vk::raii::Fence fence{Vulkan::device(), vk::FenceCreateInfo{}};

			auto graphicsQueue = Vulkan::getGraphicsQueue();
			graphicsQueue.resource.submit(submitInfo, *fence);

			auto &device = Vulkan::device();

			auto res = device.waitForFences(*fence, true, UINT64_MAX);
			if (res != vk::Result::eSuccess) {
				throw std::runtime_error(std::format("Failed finishing the command buffer for command queue, error {}", vk::to_string(res)));
			}
		}
	};
}// namespace Engine