#include "instance.hpp"
#include "utils.hpp"
#include "vulkanIncludes.hpp"

namespace Engine {
	struct Buffer {
		vk::raii::Buffer buffer;
		vk::raii::DeviceMemory memory;
		void *mappedMemory;

		struct Args {
			Instance &instance;
			size_t size;
			vk::BufferUsageFlags usage;
		};

		Buffer(const Args &args)
			: buffer(args.instance.device, vk::BufferCreateInfo{
											   .size = args.size,
											   .usage = args.usage,
											   .sharingMode = vk::SharingMode::eExclusive,
										   }),
			  memory(args.instance.device, vk::MemoryAllocateInfo{
											   .allocationSize = buffer.getMemoryRequirements().size,
											   .memoryTypeIndex = findMemoryType(buffer.getMemoryRequirements().memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, args.instance),
										   }),
			  mappedMemory(memory.mapMemory(0, buffer.getMemoryRequirements().size)) {
			buffer.bindMemory(*memory, 0);
		}
	};
}// namespace Engine