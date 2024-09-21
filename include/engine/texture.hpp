#pragma once
#include "utils.hpp"
#include "vulkan.hpp"
#include "vulkanIncludes.hpp"
#include <stdexcept>


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
			uint32_t width;
			uint32_t height;
			uint32_t channels;
		};

		Texture(const Args &args)
			: image(createImage(args)),
			  memory(createMemory()),
			  sampler(createSampler()),
			  view(createImageView(args)),
			  width(args.width),
			  height(args.height),
			  channels(args.channels) {
			auto reqs = image.getMemoryRequirements();
			mappedMemory = memory.mapMemory(0, reqs.size);

			auto props = Vulkan::findQueueFamilies(Vulkan::physicalDevice());

			vk::CommandPoolCreateInfo poolInfo{
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = props.graphicsFamily.value(),
			};

			auto commandPool = Vulkan::device().resource.createCommandPool(poolInfo);

			vk::raii::CommandBuffer cmd = std::move(
				Vulkan::device().resource.allocateCommandBuffers(
											 vk::CommandBufferAllocateInfo{
												 .commandPool = *commandPool,
												 .level = vk::CommandBufferLevel::ePrimary,
												 .commandBufferCount = 1,
											 }
				)
					.front()
			);

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

			vk::raii::Fence fence{Vulkan::device().resource, vk::FenceCreateInfo{}};

			auto graphicsQueue = Vulkan::getGraphicsQueue();
			graphicsQueue.resource.submit(submitInfo, *fence);

			auto res = Vulkan::device().resource.waitForFences(*fence, true, 100000000);
			if (res != vk::Result::eSuccess) {
				throw std::runtime_error("Texture creation failed :(");
			}
		}

		[[nodiscard]] vk::raii::ImageView createImageView(const Args &args) const {
			vk::ImageViewCreateInfo createInfo{
				.image = *image,
				.viewType = vk::ImageViewType::e2D,
				.format = formatFromChannels(args.channels),
				.components = {
					.r = vk::ComponentSwizzle::eR,
					.g = vk::ComponentSwizzle::eG,
					.b = vk::ComponentSwizzle::eB,
					.a = vk::ComponentSwizzle::eA,
				},
				.subresourceRange{
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			};

			return {Vulkan::device().resource, createInfo};
		}

		[[nodiscard]] vk::raii::Sampler createSampler() const {
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

			return {Vulkan::device().resource, createInfo};
		}

		[[nodiscard]] vk::raii::DeviceMemory createMemory() const {
			auto reqs = image.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo{
				.allocationSize = reqs.size,
				.memoryTypeIndex = findMemoryType(reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
			};

			return {Vulkan::device().resource, allocInfo};
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

			return {Vulkan::device().resource, createInfo};
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