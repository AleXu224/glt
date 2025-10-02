#pragma once

#include "buffer.hpp"
#include "frame.hpp"
#include "instance.hpp"
#include "utils.hpp"
#include "vulkanIncludes.hpp"
#include <array>
#include <cstring>
#include <functional>
#include <print>
#include <ranges>
#include <vulkan/vulkan_enums.hpp>


namespace Engine {
	struct Ubo {
		glm::mat4 model = glm::mat4(1.f);
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

		vk::raii::DescriptorSetLayout descriptorSetLayout;

		std::array<Buffer, FrameBuffer> buffers;

		vk::raii::DescriptorPool descriptorPool;
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		Instance &instance;

		Uniform(const Args &args)
			: descriptorSetLayout(createSetLayout()),
			  buffers(generateArray<Buffer, FrameBuffer>([&](auto) {
				  return Buffer{Buffer::Args{
					  .size = sizeof(T),
					  .usage = vk::BufferUsageFlagBits::eUniformBuffer,
				  }};
			  })),
			  descriptorPool(createDescriptorPool()),
			  descriptorSets(createDescriptorSets()),
			  instance(args.instance) {
			for (auto i: std::views::iota(0ull, FrameBuffer)) {
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

				Vulkan::device().resource.updateDescriptorSets(descriptorWrite, {});
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
				Vulkan::device().resource,
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

		static vk::raii::DescriptorPool createDescriptorPool() {
			vk::DescriptorPoolSize size{
				.type = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = FrameBuffer,
			};

			vk::DescriptorPoolCreateInfo createInfo{
				.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				.maxSets = FrameBuffer,
				.poolSizeCount = 1,
				.pPoolSizes = &size,
			};

			return Vulkan::device().resource.createDescriptorPool(createInfo);
		}

		std::vector<vk::raii::DescriptorSet> createDescriptorSets() {
			auto setLayouts = generateArray<vk::DescriptorSetLayout, FrameBuffer>([&](size_t /*i*/) -> vk::DescriptorSetLayout {
				return *descriptorSetLayout;
			});

			vk::DescriptorSetAllocateInfo allocInfo{
				.descriptorPool = *descriptorPool,
				.descriptorSetCount = FrameBuffer,
				.pSetLayouts = setLayouts.data(),
			};

			return Vulkan::device().resource.allocateDescriptorSets(allocInfo);
		}
	};
}// namespace Engine