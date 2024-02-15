#pragma once
#include "utils.hpp"
#include "vulkanIncludes.hpp"
#include <mutex>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>

inline std::mutex queue_mutex;

namespace Engine {
	struct Texture {
		vk::raii::Image image;
		vk::raii::DeviceMemory memory;
		void *mappedMemory;
		vk::raii::Sampler sampler;
		vk::raii::ImageView view;

		uint32_t width;
		uint32_t height;
		uint32_t channels;

		struct Args {
			Instance &instance;
			uint32_t width;
			uint32_t height;
			uint32_t channels;
		};

		Texture(const Args &args)
			: image(createImage(args)),
			  memory(createMemory(args)),
			  sampler(createSampler(args)),
			  view(createImageView(args)),
			  width(args.width),
			  height(args.height),
			  channels(args.channels) {
			auto reqs = image.getMemoryRequirements();
			mappedMemory = memory.mapMemory(0, reqs.size);

			auto props = args.instance.findQueueFamilies(args.instance.physicalDevice);

			vk::CommandPoolCreateInfo poolInfo{
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = props.graphicsFamily.value(),
			};

			auto commandPool = args.instance.device.createCommandPool(poolInfo);

			vk::raii::CommandBuffer cmd = std::move(args.instance.device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
																									.commandPool = *commandPool,
																									.level = vk::CommandBufferLevel::ePrimary,
																									.commandBufferCount = 1,
																								})
														.front());

			cmd.begin({});

			vk::ImageSubresourceRange subresourceRange{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			};

			vk::ImageMemoryBarrier imageMemBarrier{
				.srcAccessMask = vk::AccessFlagBits::eHostWrite,
				.dstAccessMask = vk::AccessFlagBits::eShaderRead,
				.oldLayout = vk::ImageLayout::ePreinitialized,
				.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
				.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
				.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
				.image = *image,
				.subresourceRange = subresourceRange,
			};

			cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, imageMemBarrier);
			cmd.end();

			vk::SubmitInfo submitInfo{
				.commandBufferCount = 1,
				.pCommandBuffers = &*cmd,
			};

			vk::raii::Fence fence{args.instance.device, vk::FenceCreateInfo{}};

			queue_mutex.lock();
			args.instance.graphicsQueue.submit(submitInfo, *fence);

			auto res = args.instance.device.waitForFences(*fence, true, 100000000);
			if (res != vk::Result::eSuccess) {
				throw std::runtime_error("Texture creation failed :(");
			}
			queue_mutex.unlock();
		}

		vk::raii::ImageView createImageView(const Args &args) const {
			vk::ImageViewCreateInfo createInfo{
				.image = *image,
				.viewType = vk::ImageViewType::e2D,
				.format = formatFromChannels(args.channels),
				.components = {
					vk::ComponentSwizzle::eR,
					vk::ComponentSwizzle::eG,
					vk::ComponentSwizzle::eB,
					vk::ComponentSwizzle::eA,
				},
				.subresourceRange{
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			};

			return {args.instance.device, createInfo};
		}

		vk::raii::Sampler createSampler(const Args &args) const {
			image.bindMemory(*memory, 0);

			vk::SamplerCreateInfo createInfo{
				.magFilter = vk::Filter::eLinear,
				.minFilter = vk::Filter::eLinear,
				.mipmapMode = vk::SamplerMipmapMode::eLinear,
				.addressModeU = vk::SamplerAddressMode::eRepeat,
				.addressModeV = vk::SamplerAddressMode::eRepeat,
				.addressModeW = vk::SamplerAddressMode::eRepeat,
				.mipLodBias = 0.f,
				.anisotropyEnable = false,
				.maxAnisotropy = 1.f,
				.compareOp = vk::CompareOp::eNever,
				.minLod = 0.f,
				.maxLod = 0.f,
				.borderColor = vk::BorderColor::eFloatTransparentBlack,
			};

			return {args.instance.device, createInfo};
		}

		vk::raii::DeviceMemory createMemory(const Args &args) const {
			auto reqs = image.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo{
				.allocationSize = reqs.size,
				.memoryTypeIndex = findMemoryType(reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, args.instance),
			};

			return {args.instance.device, allocInfo};
		}

		static vk::raii::Image createImage(const Args &args) {
			vk::ImageCreateInfo createInfo{
				.imageType = vk::ImageType::e2D,
				.format = formatFromChannels(args.channels),
				.extent{
					.width = args.width,
					.height = args.height,
					.depth = 1,
				},
				.mipLevels = 1,
				.arrayLayers = 1,
				// Using linear since we won't be using a staging buffer
				.samples = vk::SampleCountFlagBits::e1,
				.tiling = vk::ImageTiling::eLinear,
				.usage = vk::ImageUsageFlagBits::eSampled,
				.sharingMode = vk::SharingMode::eExclusive,
				.initialLayout = vk::ImageLayout::ePreinitialized,
			};

			return {args.instance.device, createInfo};
		}

		static vk::Format formatFromChannels(uint32_t channels) {
			switch (channels) {
				case 4: {
					return vk::Format::eR8G8B8A8Unorm;
				}
				case 3: {
					throw std::runtime_error("3 channel textures are not supported");
					// return vk::Format::eR8G8B8Unorm;
				}
				case 2: {
					return vk::Format::eR8G8Unorm;
				}
				case 1: {
					return vk::Format::eR8Unorm;
				}
				default: {
					throw std::runtime_error("Unsupported number of channels specified");
				}
			}
		}
	};
}// namespace Engine