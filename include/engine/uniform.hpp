#pragma once

#include "buffer.hpp"
#include "frame.hpp"
#include "instance.hpp"
#include "vulkanIncludes.hpp"
#include <functional>
#include <ranges>
#include <vulkan/vulkan_enums.hpp>


namespace Engine {
	struct PushConstant {
		glm::mat4 model = glm::mat4(1.f);
	};

	struct Ubo {
		glm::mat4 view;
	};

	struct UniformData {
		vk::raii::DescriptorSetLayout &descriptorSetLayout;
		const std::function<void(vk::raii::PipelineLayout &)> bind;
	};

	template<class T>
	struct Uniform {
		struct Args {
			Instance &instance;
		};

		Instance &instance;

		vk::raii::DescriptorSetLayout descriptorSetLayout;

		std::vector<Buffer> buffers;

		vk::raii::DescriptorPool descriptorPool;
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		Uniform(const Args &args)
			: instance(args.instance),
			  descriptorSetLayout(createSetLayout()),
			  buffers([&]() {
				  std::vector<Buffer> ret;
				  for (auto _: std::views::iota(0ull, instance.frames.size())) {
					  ret.emplace_back(Buffer::Args{
						  .size = sizeof(T),
						  .usage = vk::BufferUsageFlagBits::eUniformBuffer,
					  });
				  }
				  return ret;
			  }()),
			  descriptorPool(createDescriptorPool()),
			  descriptorSets(createDescriptorSets()) {
			for (auto i: std::views::iota(0ull, instance.frames.size())) {
				auto &buffer = buffers.at(i).buffer;

				vk::DescriptorBufferInfo bufferInfo{
					.buffer = *buffer,
					.offset = 0,
					.range = sizeof(T),
				};

				vk::WriteDescriptorSet descriptorWrite{
					.dstSet = *descriptorSets.at(i),
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eUniformBuffer,
					.pBufferInfo = &bufferInfo,
				};

				Vulkan::device().updateDescriptorSets(descriptorWrite, {});
			}
		}

		vk::DescriptorSet getDescriptorSet() {
			return *descriptorSets.at(instance.currentFrame.get().index);
		}

		void bind(vk::raii::PipelineLayout &layout) {
			instance.currentFrame.get().commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *layout, 0, *descriptorSets.at(instance.currentFrame.get().index), {});
		}

		T &getData() {
			return *reinterpret_cast<T *>(buffers[instance.currentFrame.get().index].mappedMemory);
		}

		operator UniformData() {
			return {
				.descriptorSetLayout = descriptorSetLayout,
				.bind = [&](vk::raii::PipelineLayout &layout) {
					return bind(layout);
				},
			};
		}

	private:
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

		static vk::DescriptorSetLayoutBinding getDescriptorSetLayout(uint32_t binding = 0, uint32_t count = 1) {
			return {
				.binding = binding,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = count,
				.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
			};
		}

		vk::raii::DescriptorPool createDescriptorPool() {
			vk::DescriptorPoolSize size{
				.type = vk::DescriptorType::eUniformBuffer,
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

		std::vector<vk::raii::DescriptorSet> createDescriptorSets() {
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