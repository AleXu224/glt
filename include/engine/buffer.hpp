#pragma once

#include "utils.hpp"
#include "vulkan.hpp"
#include "vulkanIncludes.hpp"

namespace Engine {
	struct Buffer {
		vk::raii::Buffer buffer;
		vk::raii::DeviceMemory memory;
		void *mappedMemory;

		struct Args {
			size_t size;
			vk::BufferUsageFlags usage;
		};

		Buffer(const Args &args)
			: buffer(Vulkan::device().resource, vk::BufferCreateInfo{
													.size = args.size,
													.usage = args.usage,
													.sharingMode = vk::SharingMode::eExclusive,
												}),
			  memory(Vulkan::device().resource, vk::MemoryAllocateInfo{
													.allocationSize = buffer.getMemoryRequirements().size,
													.memoryTypeIndex = findMemoryType(buffer.getMemoryRequirements().memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
												}),
			  mappedMemory(memory.mapMemory(0, buffer.getMemoryRequirements().size)) {
			buffer.bindMemory(*memory, 0);
		}
	};
}// namespace Engine