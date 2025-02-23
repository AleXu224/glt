#pragma once
#include "vulkanIncludes.hpp"


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
		uint32_t mipLevels;

		struct Args {
			uint32_t width;
			uint32_t height;
			uint32_t channels;
			uint32_t mipLevels = 1;
		};

		void transitionLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask);

		Texture(const Args &args);

		[[nodiscard]] vk::raii::ImageView createImageView(const Args &args) const;

		[[nodiscard]] vk::raii::Sampler createSampler(const Args &args) const;

		[[nodiscard]] vk::raii::DeviceMemory createMemory() const;

		void generateMipmaps();

		void makeTextureWriteable();

		void returnTexture();

		static vk::raii::Image createImage(const Args &args);

		static vk::Format formatFromChannels(uint32_t channels);
	};
}// namespace Engine