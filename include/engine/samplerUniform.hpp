#pragma once

#include "frame.hpp"
#include "instance.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include "vulkanIncludes.hpp"
#include <array>
#include <cstring>
#include <functional>
#include <print>
#include <ranges>
#include <thread>
#include <utility>


namespace Engine {
	struct SamplerUniform {
		struct Args {
			Instance &instance;
			Texture::Args textureArgs;
		};

		Texture texture;

		vk::raii::DescriptorSetLayout descriptorSetLayout;

		vk::raii::DescriptorPool descriptorPool;
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		Instance &instance;

		SamplerUniform(const SamplerUniform &) = delete;
		SamplerUniform(SamplerUniform &&) = delete;
		SamplerUniform &operator=(const SamplerUniform &) = delete;
		SamplerUniform &operator=(SamplerUniform &&) = delete;
		SamplerUniform(const Args &args)
			: texture(args.textureArgs),
			  descriptorSetLayout(createSetLayout(args)),
			  descriptorPool(createDescriptorPool(args)),
			  descriptorSets(createDescriptorSets(args)),
			  instance(args.instance) {
			for (auto i: std::views::iota(0ULL, FrameBuffer)) {
				vk::DescriptorImageInfo bufferInfo{
					.sampler = *texture.sampler,
					.imageView = *texture.view,
					.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
				};

				vk::WriteDescriptorSet descriptorWrite{
					.dstSet = *descriptorSets.at(i),
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eCombinedImageSampler,
					.pImageInfo = &bufferInfo,
				};

				args.instance.device.updateDescriptorSets(descriptorWrite, {});
			}
		}

		~SamplerUniform() {
			auto thread = std::thread([&instance = instance, texture = std::move(texture), descriptorPool = std::move(descriptorPool), descriptorSets = std::move(descriptorSets)] {
				// Wait for the device to be done with the descriptor sets
				graphicsQueueMutex.lock();
				instance.device.waitIdle();
				graphicsQueueMutex.unlock();
			});
			thread.detach();
		}

		vk::DescriptorSet getDescriptorSet() const {
			return *descriptorSets.at(instance.currentFrame.get().index);
		}

		void bind(vk::raii::PipelineLayout &layout) {
			instance.currentFrame.get().commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *layout, 0, *descriptorSets.at(instance.currentFrame.get().index), {});
		}

		static vk::raii::DescriptorSetLayout createSetLayout(const Args &args) {
			auto layoutBinding = getDescriptorSetLayout();

			vk::DescriptorSetLayoutCreateInfo createInfo{
				.bindingCount = 1,
				.pBindings = &layoutBinding,
			};

			return vk::raii::DescriptorSetLayout{
				args.instance.device,
				createInfo,
			};
		}

	private:
		static vk::DescriptorSetLayoutBinding getDescriptorSetLayout(uint32_t binding = 0, uint32_t count = 1) {
			return {
				.binding = binding,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = count,
				.stageFlags = vk::ShaderStageFlagBits::eFragment,
			};
		}

		static vk::raii::DescriptorPool createDescriptorPool(const Args &args) {
			vk::DescriptorPoolSize size{
				.type = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = FrameBuffer,
			};

			vk::DescriptorPoolCreateInfo createInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = FrameBuffer,
				.poolSizeCount = 1,
				.pPoolSizes = &size,
			};

			return args.instance.device.createDescriptorPool(createInfo);
		}

		[[nodiscard]] std::vector<vk::raii::DescriptorSet> createDescriptorSets(const Args &args) const {
			auto setLayouts = generateArray<vk::DescriptorSetLayout, FrameBuffer>([&](size_t /*i*/) -> vk::DescriptorSetLayout {
				return *descriptorSetLayout;
			});

			vk::DescriptorSetAllocateInfo allocInfo{
				.descriptorPool = *descriptorPool,
				.descriptorSetCount = FrameBuffer,
				.pSetLayouts = setLayouts.data(),
			};

			return args.instance.device.allocateDescriptorSets(allocInfo);
		}
	};
}// namespace Engine