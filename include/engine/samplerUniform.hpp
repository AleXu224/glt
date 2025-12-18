#pragma once

#include "frame.hpp"
#include "instance.hpp"
#include "texture.hpp"
#include "vulkan.hpp"
#include "vulkanIncludes.hpp"
#include <functional>
#include <ranges>
#include <utility>


namespace Engine {
	struct SamplerUniform : public std::enable_shared_from_this<SamplerUniform> {
		struct Args {
			Instance &instance;
			std::shared_ptr<Texture> texture;
		};

		Instance &instance;

		std::shared_ptr<Texture> texture;

		vk::raii::DescriptorSetLayout descriptorSetLayout;

		vk::raii::DescriptorPool descriptorPool;
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		SamplerUniform(const SamplerUniform &) = delete;
		SamplerUniform(SamplerUniform &&) = delete;
		SamplerUniform &operator=(const SamplerUniform &) = delete;
		SamplerUniform &operator=(SamplerUniform &&) = delete;
		SamplerUniform(const Args &args)
			: instance(args.instance),
			  texture(args.texture),
			  descriptorSetLayout(createSetLayout()),
			  descriptorPool(createDescriptorPool()),
			  descriptorSets(createDescriptorSets()) {
			for (auto i: std::views::iota(0ULL, instance.frames.size())) {
				vk::DescriptorImageInfo bufferInfo{
					.sampler = *texture->sampler,
					.imageView = *texture->view,
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

				Vulkan::device().updateDescriptorSets(descriptorWrite, {});
			}
		}

		~SamplerUniform() {
			struct DataHolder {
				vk::raii::DescriptorPool descriptorPool;
				std::vector<vk::raii::DescriptorSet> descriptorSets;
				std::shared_ptr<Texture> texture;
			};

			auto data = std::make_shared<DataHolder>(DataHolder{
				.descriptorPool = std::move(descriptorPool),
				.descriptorSets = std::move(descriptorSets),
				.texture = std::move(texture),
			});

			// Delay destruction to the next frame to avoid issues with in-flight frames
			instance.nextFrameTasks.emplace_back([data] {});
		}

		vk::DescriptorSet getDescriptorSet() const {
			return *descriptorSets.at(instance.currentFrame.get().index);
		}

		void bind(vk::raii::PipelineLayout &layout) {
			instance.currentFrame.get().commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *layout, 0, *descriptorSets.at(instance.currentFrame.get().index), {});
		}

		static vk::raii::DescriptorSetLayout createSetLayout() {
			auto layoutBinding = getDescriptorSetLayout();

			vk::DescriptorSetLayoutCreateInfo createInfo{
				.bindingCount = 1,
				.pBindings = &layoutBinding,
			};

			return vk::raii::DescriptorSetLayout{
				Vulkan::device(),
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

		vk::raii::DescriptorPool createDescriptorPool() const {
			vk::DescriptorPoolSize size{
				.type = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = static_cast<uint32_t>(instance.frames.size()),
			};

			vk::DescriptorPoolCreateInfo createInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = static_cast<uint32_t>(instance.frames.size()),
				.poolSizeCount = 1,
				.pPoolSizes = &size,
			};

			return Vulkan::device().createDescriptorPool(createInfo);
		}

		[[nodiscard]] std::vector<vk::raii::DescriptorSet> createDescriptorSets() const {
			std::vector<vk::DescriptorSetLayout> setLayouts(instance.frames.size(), *descriptorSetLayout);

			vk::DescriptorSetAllocateInfo allocInfo{
				.descriptorPool = *descriptorPool,
				.descriptorSetCount = static_cast<uint32_t>(instance.frames.size()),
				.pSetLayouts = setLayouts.data(),
			};

			return Vulkan::device().allocateDescriptorSets(allocInfo);
		}
	};
}// namespace Engine