#pragma once

#include "vulkan.hpp"

namespace Engine {
	struct CommandQueue {
	private:
		static inline std::mutex mtx{};
		// Keeps Command Pools and Command Buffers alive until the end of the frame
		static inline std::list<std::pair<vk::raii::CommandPool, vk::raii::CommandBuffer>> storage{};
		static inline std::vector<vk::raii::CommandBuffer *> commandBuffers{};

	public:
		static inline std::pair<vk::raii::CommandPool, vk::raii::CommandBuffer> &pushStorage(std::pair<vk::raii::CommandPool, vk::raii::CommandBuffer> &&pair) {
			std::scoped_lock lock{mtx};
			auto &ret = storage.emplace_back(std::move(pair));
			return ret;
		}

		static inline void pushCommandBuffer(vk::raii::CommandBuffer *cmd) {
			std::scoped_lock lock{mtx};
			commandBuffers.push_back(cmd);
		}

		static inline void clear() {
			std::scoped_lock lock{mtx};
			storage.clear();
			commandBuffers.clear();
		}

		[[nodiscard]] static inline std::scoped_lock<std::mutex> acquireLock() {
			return std::scoped_lock<std::mutex>{mtx};
		}

		static inline void frameEnd() {
			std::scoped_lock lock{mtx};

			std::vector<vk::CommandBuffer> cmds;
			cmds.reserve(commandBuffers.size());
			for (auto cmdPtr: commandBuffers) {
				cmds.push_back(*cmdPtr);
			}

			vk::SubmitInfo submitInfo{
				.commandBufferCount = static_cast<uint32_t>(cmds.size()),
				.pCommandBuffers = cmds.data(),
			};

			vk::raii::Fence fence{Vulkan::device(), vk::FenceCreateInfo{}};

			auto graphicsQueue = Vulkan::getGraphicsQueue();
			graphicsQueue.resource.submit(submitInfo, *fence);

			auto &device = Vulkan::device();

			auto res = device.waitForFences(*fence, true, 100000000);
			if (res != vk::Result::eSuccess) {
				throw std::runtime_error("Failed finishing the command buffer");
			}

			storage.clear();
			commandBuffers.clear();
		}
	};
}// namespace Engine