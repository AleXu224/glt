#pragma once
#include "vulkanIncludes.hpp"

#include "functional"

namespace Engine {
	struct TextureWriter {
		struct Args {
			bool first = false;
			bool makeReadable = false;
		};
		void *memory;

		TextureWriter(const TextureWriter &) = delete;
		TextureWriter &operator=(const TextureWriter &) = delete;
		TextureWriter(TextureWriter &&);
		TextureWriter &operator=(TextureWriter &&);
		
		TextureWriter(
			uint32_t width,
			uint32_t height,
			vk::raii::Image &image,
			std::function<void(vk::ImageLayout, vk::ImageLayout, vk::PipelineStageFlags, vk::PipelineStageFlags)> transitionFunc,
			Args args
		);
		void write();
		~TextureWriter();
		bool valid = false;

	private:
		uint32_t width;
		uint32_t height;
		vk::raii::Image *image;
		vk::raii::Buffer stagingBuffer = nullptr;
		vk::raii::DeviceMemory stagingMemory = nullptr;
		std::function<void(vk::ImageLayout, vk::ImageLayout, vk::PipelineStageFlags, vk::PipelineStageFlags)> transitionFunc;
	};

	struct Texture {
		vk::raii::Image image;
		vk::raii::DeviceMemory memory;
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

		static vk::raii::Image createImage(const Args &args);

		static vk::Format formatFromChannels(uint32_t channels);

		TextureWriter getWriter(TextureWriter::Args args = {});
	};
}// namespace Engine