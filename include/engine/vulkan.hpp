#pragma once

#include "loader.hpp"
#include "vulkanIncludes.hpp"
#include <mutex>

namespace Engine {
	template<class T>
	struct LockedResource {
		std::scoped_lock<std::mutex> mtx;
		T resource;
	};
	struct Vulkan {
		static inline bool validationLayersAvailable = true;

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			[[nodiscard]] bool isComplete() const {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};
		[[nodiscard]] static Engine::Vulkan::QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice &physicalDevice);

		[[nodiscard]] static LockedResource<vk::raii::Queue> getGraphicsQueue();
		[[nodiscard]] static LockedResource<vk::raii::Queue> getPresentQueue();
		[[nodiscard]] static std::pair<vk::raii::CommandPool, vk::raii::CommandBuffer> makeCommandBuffer();
		static void finishCommandBuffer(vk::raii::CommandBuffer &cmd);

		static inline std::mutex graphicsQueueMtx;

		static std::optional<DynamicLoader> &loader();
		static std::optional<DynamicLoader> &fallbackLoader();
		static vk::raii::Context &context();
		static bool checkValidationLayers();
		static vk::raii::Instance &instance();
		static vk::raii::PhysicalDevice &physicalDevice();
		static LockedResource<vk::raii::Device &> device();
	};
}// namespace Engine