#include "engine/texture.hpp"

#include "engine/utils.hpp"
#include "vulkan.hpp"

void Engine::Texture::transitionLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask) {
	auto [pool, cmd] = Vulkan::makeCommandBuffer();

	cmd.begin({});

	vk::ImageSubresourceRange subresourceRange{
		.aspectMask = vk::ImageAspectFlagBits::eColor,
		.baseMipLevel = 0,
		.levelCount = mipLevels,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	vk::ImageMemoryBarrier imageMemBarrier{
		.srcAccessMask = vk::AccessFlagBits::eHostWrite,
		.dstAccessMask = vk::AccessFlagBits::eShaderRead,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = *image,
		.subresourceRange = subresourceRange,
	};

	cmd.pipelineBarrier(srcStageMask, dstStageMask, {}, nullptr, nullptr, imageMemBarrier);

	cmd.end();

	Vulkan::finishCommandBuffer(cmd);
}

Engine::Texture::Texture(const Args &args)
	: image(createImage(args)),
	  memory(createMemory()),
	  sampler(createSampler(args)),
	  view(createImageView(args)),
	  width(args.width),
	  height(args.height),
	  channels(args.channels),
	  mipLevels(args.mipLevels) {
	auto reqs = image.getMemoryRequirements();
	mappedMemory = memory.mapMemory(0, reqs.size);

	transitionLayout(
		vk::ImageLayout::ePreinitialized,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eHost,
		vk::PipelineStageFlagBits::eFragmentShader
	);
}

vk::raii::ImageView Engine::Texture::createImageView(const Args &args) const {
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
			.levelCount = args.mipLevels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	return {Vulkan::device().resource, createInfo};
}

vk::raii::Sampler Engine::Texture::createSampler(const Args &args) const {
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
		.maxLod = static_cast<float>(args.mipLevels),
		.borderColor = vk::BorderColor::eFloatTransparentBlack,
	};

	return {Vulkan::device().resource, createInfo};
}

vk::raii::DeviceMemory Engine::Texture::createMemory() const {
	auto reqs = image.getMemoryRequirements();

	vk::MemoryAllocateInfo allocInfo{
		.allocationSize = reqs.size,
		.memoryTypeIndex = findMemoryType(reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
	};

	return {Vulkan::device().resource, allocInfo};
}

void Engine::Texture::generateMipmaps() {
	int32_t mipWidth = width;
	int32_t mipHeight = height;

	auto [pool, cmd] = Vulkan::makeCommandBuffer();

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
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = *image,
		.subresourceRange = subresourceRange,
	};

	for (uint32_t i = 1; i < mipLevels; i++) {
		imageMemBarrier.subresourceRange.baseMipLevel = i - 1;
		imageMemBarrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageMemBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		imageMemBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, imageMemBarrier);

		imageMemBarrier.subresourceRange.baseMipLevel = i;
		imageMemBarrier.oldLayout = vk::ImageLayout::eUndefined;
		imageMemBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemBarrier.srcAccessMask = static_cast<vk::AccessFlags>(0);
		imageMemBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, imageMemBarrier);

		vk::ImageBlit blit{
			.srcSubresource{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.mipLevel = i - 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets = std::array{
				vk::Offset3D{0, 0, 0},
				vk::Offset3D{mipWidth, mipHeight, 1},
			},
			.dstSubresource{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.mipLevel = i,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets = std::array{
				vk::Offset3D{0, 0, 0},
				vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1},
			},
		};

		cmd.blitImage(
			*this->image,
			vk::ImageLayout::eTransferSrcOptimal,
			*this->image,
			vk::ImageLayout::eTransferDstOptimal,
			std::array{
				blit
			},
			vk::Filter::eLinear
		);

		imageMemBarrier.subresourceRange.baseMipLevel = i - 1;
		imageMemBarrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		imageMemBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageMemBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		imageMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, imageMemBarrier);

		imageMemBarrier.subresourceRange.baseMipLevel = i;
		imageMemBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		imageMemBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageMemBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		imageMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, imageMemBarrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	cmd.end();

	Vulkan::finishCommandBuffer(cmd);
}

vk::raii::Image Engine::Texture::createImage(const Args &args) {
	vk::ImageCreateInfo createInfo{
		.imageType = vk::ImageType::e2D,
		.format = formatFromChannels(args.channels),
		.extent{
			.width = args.width,
			.height = args.height,
			.depth = 1,
		},
		.mipLevels = args.mipLevels,
		.arrayLayers = 1,
		// Using linear since we won't be using a staging buffer
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eLinear,
		.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
		.sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::ePreinitialized,
	};

	return {Vulkan::device().resource, createInfo};
}

vk::Format Engine::Texture::formatFromChannels(uint32_t channels) {
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
