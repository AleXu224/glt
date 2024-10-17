#pragma once
#include "frame.hpp"
#include "instance.hpp"
#include "samplerUniform.hpp"
#include "shader.hpp"
#include "uniform.hpp"
#include "vulkanIncludes.hpp"
#include <array>
#include <cstring>
#include <functional>
#include <memory>
#include <print>
#include <span>
#include <tuple>
#include <utility>
#include <vector>
#include <vulkan/vulkan_enums.hpp>


namespace Engine {
	template<class Vertex, bool hasTexture = false, class... Uniforms>
	struct Pipeline {
		struct Args {
			size_t vertexBufferSize = 1024ull * 4;
			size_t IndexBufferSize = 1024ull * 6;
			const std::span<const char> vertexShader;
			const std::span<const char> fragmentShader;
			Instance &instance;
		};

		vk::raii::PipelineLayout layout = nullptr;
		vk::raii::Pipeline pipeline = nullptr;
		Shader fragmentShader;
		Shader vertexShader;
		Uniform<Ubo> basicUniform;
		std::tuple<Uniform<Uniforms>...> uniforms;

		squi::VoidObserver frameBeginListener;
		squi::VoidObserver frameEndListener;

		template<size_t Ind>
		constexpr auto &getUniform() {
			return std::get<Ind>(uniforms);
		}

		size_t lastVertexBufferIndex = 0;
		size_t vertexBufferIndex = 0;
		size_t vertexArrIndex = 0;

		size_t lastIndexBufferIndex = 0;
		size_t indexBufferIndex = 0;
		size_t indexArrIndex = 0;

		[[nodiscard]] Buffer &getCurrentVertexBuffer() {
			return *vertexBuffers.at(vertexArrIndex);
		}
		[[nodiscard]] Buffer &getCurrentIndexBuffer() {
			return *indexBuffers.at(indexArrIndex);
		}

		std::vector<std::unique_ptr<Buffer>> vertexBuffers{};
		std::vector<std::unique_ptr<Buffer>> indexBuffers{};

		Pipeline(const Pipeline &) = delete;
		Pipeline(Pipeline &) = delete;
		Pipeline &operator=(const Pipeline &) = delete;
		Pipeline(const Pipeline &&) = delete;
		Pipeline(Pipeline &&) = delete;
		Pipeline &operator=(Pipeline &&) = delete;
		~Pipeline() = default;

		Pipeline(const Args &args)
			: fragmentShader(Vulkan::device().resource, args.fragmentShader),
			  vertexShader(Vulkan::device().resource, args.vertexShader),
			  basicUniform({.instance = args.instance}),
			  uniforms([&]() -> std::tuple<Uniform<Uniforms>...> {
				  return [&]<size_t... I>(const std::index_sequence<I...> &) {
					  return std::tuple<Uniform<Uniforms>...>{
						  (Uniform<Uniforms>{{.instance = args.instance}})...,
					  };
				  }(std::make_index_sequence<sizeof...(Uniforms)>());
			  }()),
			  frameBeginListener(args.instance.frameBeginEvent.observe([this] {
				  float width = instance.swapChainExtent.width;
				  float height = instance.swapChainExtent.height;

				  basicUniform.getData().view = glm::mat4{
					  1.0f / (width / 2.f), 0.0f, 0.0f, 0.0f,
					  0.0f, 1.0f / (height / 2.f), 0.0f, 0.0f,
					  -1.0f, -1.0f, 1.0f, 0.0f,
					  0.0f, 0.0f, 0.0f, 1.0f
				  };
			  })),
			  frameEndListener(args.instance.frameEndEvent.observe([this] {
				  lastVertexBufferIndex = 0;
				  vertexBufferIndex = 0;
				  vertexArrIndex = 0;

				  lastIndexBufferIndex = 0;
				  indexBufferIndex = 0;
				  indexArrIndex = 0;

				  binds = 0;
			  })),
			  vertexBufferSize(args.vertexBufferSize),
			  indexBufferSize(args.IndexBufferSize),
			  instance(args.instance) {
			float width = instance.swapChainExtent.width;
			float height = instance.swapChainExtent.height;

			basicUniform.getData().view = glm::mat4{
				1.0f / (width / 2.f), 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f / (height / 2.f), 0.0f, 0.0f,
				-1.0f, -1.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};
			vertexBuffers.emplace_back(std::make_unique<Buffer>(Buffer::Args{
				.size = sizeof(Vertex) * args.vertexBufferSize,
				.usage = vk::BufferUsageFlagBits::eVertexBuffer,
			}));
			indexBuffers.emplace_back(std::make_unique<Buffer>(Buffer::Args{
				.size = sizeof(uint16_t) * args.IndexBufferSize,
				.usage = vk::BufferUsageFlagBits::eIndexBuffer,
			}));
			vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = *vertexShader.module,
				.pName = "main",
			};

			vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = *fragmentShader.module,
				.pName = "main",
			};

			std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{vertShaderStageInfo, fragShaderStageInfo};

			std::vector<vk::DynamicState> dynamicStates = {
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor,
			};

			vk::PipelineDynamicStateCreateInfo dynamicState{
				.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
				.pDynamicStates = dynamicStates.data(),
			};

			vk::VertexInputBindingDescription bindingDescription{
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = vk::VertexInputRate::eVertex,
			};

			auto attributeDescriptions = Vertex::describe();

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
				.vertexBindingDescriptionCount = 1,
				.pVertexBindingDescriptions = &bindingDescription,
				.vertexAttributeDescriptionCount = attributeDescriptions.size(),
				.pVertexAttributeDescriptions = attributeDescriptions.data(),
			};

			vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
				.topology = vk::PrimitiveTopology::eTriangleList,
				.primitiveRestartEnable = false,
			};

			vk::Viewport viewport{
				.x = 0.f,
				.y = 0.f,
				.width = static_cast<float>(instance.swapChainExtent.width),
				.height = static_cast<float>(instance.swapChainExtent.height),
				.minDepth = 0.f,
				.maxDepth = 1.f,
			};

			vk::Rect2D scissor{
				.offset{.x = 0, .y = 0},
				.extent = instance.swapChainExtent,
			};

			vk::PipelineViewportStateCreateInfo viewportState{
				.viewportCount = 1,
				.pViewports = &viewport,
				.scissorCount = 1,
				.pScissors = &scissor,
			};

			vk::PipelineRasterizationStateCreateInfo rasterizer{
				.depthClampEnable = false,
				.rasterizerDiscardEnable = false,
				.polygonMode = vk::PolygonMode::eFill,
				// NOTE: check this if something breaks
				.cullMode = vk::CullModeFlagBits::eNone,
				.frontFace = vk::FrontFace::eClockwise,
				.depthBiasEnable = false,
				.lineWidth = 1.f,
			};

			vk::PipelineMultisampleStateCreateInfo multisampling{
				.rasterizationSamples = vk::SampleCountFlagBits::e1,
				.sampleShadingEnable = false,
				.minSampleShading = 1.f,
				.pSampleMask = nullptr,
				.alphaToCoverageEnable = false,
				.alphaToOneEnable = false,
			};

			vk::PipelineColorBlendAttachmentState colorBlendAttachment{
				.blendEnable = true,
				.srcColorBlendFactor = vk::BlendFactor::eOne,
				.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
				.colorBlendOp = vk::BlendOp::eAdd,
				.srcAlphaBlendFactor = vk::BlendFactor::eOne,
				.dstAlphaBlendFactor = vk::BlendFactor::eZero,
				.alphaBlendOp = vk::BlendOp::eAdd,
				.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
			};

			vk::PipelineColorBlendStateCreateInfo colorBlending{
				.logicOpEnable = false,
				.logicOp = vk::LogicOp::eCopy,
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachment,
				.blendConstants = {{
					0.f,
					0.f,
					0.f,
					0.f,
				}},
			};

			auto samplerUniformLayout = SamplerUniform::createSetLayout();

			auto setLayouts = [&] {
				if constexpr (hasTexture) {
					return std::apply(
						[&](auto &&...U) {
							return std::array{*basicUniform.descriptorSetLayout, *samplerUniformLayout, (*U.descriptorSetLayout)...};
						},
						uniforms
					);
				} else {
					return std::apply(
						[&](auto &&...U) {
							return std::array{*basicUniform.descriptorSetLayout, (*U.descriptorSetLayout)...};
						},
						uniforms
					);
				}
			}();


			vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
				.setLayoutCount = setLayouts.size(),
				.pSetLayouts = setLayouts.data(),
				.pushConstantRangeCount = 0,
				.pPushConstantRanges = nullptr,
			};

			layout = Vulkan::device().resource.createPipelineLayout(pipelineLayoutInfo);

			vk::GraphicsPipelineCreateInfo pipelineInfo{
				.stageCount = 2,
				.pStages = shaderStages.data(),
				.pVertexInputState = &vertexInputInfo,
				.pInputAssemblyState = &inputAssembly,
				.pViewportState = &viewportState,
				.pRasterizationState = &rasterizer,
				.pMultisampleState = &multisampling,
				.pDepthStencilState = nullptr,
				.pColorBlendState = &colorBlending,
				.pDynamicState = &dynamicState,
				.layout = *layout,
				.renderPass = *instance.renderPass,
				.subpass = 0,
				.basePipelineHandle = nullptr,
				.basePipelineIndex = -1,
			};

			pipeline = Vulkan::device().resource.createGraphicsPipeline(nullptr, pipelineInfo);
		}

		std::function<void()> currentPipelineFlush = [&] {
			this->flush(true);
		};

		uint32_t binds = 0;

		void bind() {
			if (instance.currentPipeline == this) return;
			if (instance.currentPipelineFlush) (*instance.currentPipelineFlush)();

			auto &cmd = instance.currentFrame.get().commandBuffer;
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
			cmd.bindVertexBuffers(0, *vertexBuffers.at(vertexArrIndex)->buffer, {0});
			cmd.bindIndexBuffer(*indexBuffers.at(indexArrIndex)->buffer, 0, vk::IndexType::eUint16);

			auto descriptors = std::apply([&](auto &&...U) {
				return std::array<vk::DescriptorSet, sizeof...(Uniforms) + 1>{basicUniform.getDescriptorSet(), (U.getDescriptorSet())...};
			},
										  uniforms);

			cmd.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				*layout,
				0,
				descriptors,
				{}
			);

			instance.currentPipeline = this;
			instance.currentPipelineFlush = &currentPipelineFlush;
			binds++;
		}

		void const *lastBoundSampler = nullptr;

		void bindWithSampler(const SamplerUniform &sampler) {
			auto &cmd = instance.currentFrame.get().commandBuffer;
			if (instance.currentPipeline != this || lastBoundSampler != &sampler) {
				if (instance.currentPipelineFlush) (*instance.currentPipelineFlush)();
				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
				cmd.bindVertexBuffers(0, *vertexBuffers.at(vertexArrIndex)->buffer, {0});
				cmd.bindIndexBuffer(*indexBuffers.at(indexArrIndex)->buffer, 0, vk::IndexType::eUint16);
				auto descriptors = std::apply([&](auto &&...U) {
					return std::array<vk::DescriptorSet, sizeof...(Uniforms) + 2>{basicUniform.getDescriptorSet(), sampler.getDescriptorSet(), (U.getDescriptorSet())...};
				},
											  uniforms);

				cmd.bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					*layout,
					0,
					descriptors,
					{}
				);
				lastBoundSampler = &sampler;
				binds++;
			}
			instance.currentPipeline = this;
			instance.currentPipelineFlush = &currentPipelineFlush;
		}

		[[nodiscard]] std::tuple<uint64_t, uint64_t> availableSpace() {
			return {
				static_cast<uint64_t>(vertexBufferSize) - static_cast<uint64_t>(vertexBufferIndex),
				static_cast<uint64_t>(indexBufferSize) - static_cast<uint64_t>(indexBufferIndex),
			};
		}

		[[nodiscard]] std::pair<size_t, size_t> getIndexes() {
			return {vertexBufferIndex % vertexBufferSize, indexBufferIndex % indexBufferSize};
		}

		struct Data {
			const std::span<const Vertex> vertexes;
			const std::span<const uint16_t> indexes;
		};

		void addData(const Data &data) {
			assert(data.vertexes.size() < vertexBufferSize);
			assert(data.indexes.size() < indexBufferSize);

			auto [vertexSpace, indexSpace] = availableSpace();

			size_t vertexOffset = 0;

			if (data.vertexes.size() >= vertexSpace || data.indexes.size() >= indexSpace) {
				vertexOffset = vertexBufferIndex;

				flush(false);
			}

			auto &vertexBuffer = getCurrentVertexBuffer();
			auto &indexBuffer = getCurrentIndexBuffer();

			memcpy((Vertex *) vertexBuffer.mappedMemory + vertexBufferIndex, data.vertexes.data(), data.vertexes.size() * sizeof(Vertex));
			memcpy((uint16_t *) indexBuffer.mappedMemory + indexBufferIndex, data.indexes.data(), data.indexes.size() * sizeof(uint16_t));
			if (vertexOffset) {
				for (size_t i = 0; i < data.indexes.size(); i++) {
					*((uint16_t *) indexBuffer.mappedMemory + i) -= vertexOffset;
				}
			}

			vertexBufferIndex += data.vertexes.size();
			indexBufferIndex += data.indexes.size();
			assert(indexBufferIndex <= indexBufferSize);
			assert(vertexBufferIndex <= vertexBufferSize);
		}

		void flush(bool early) {
			auto &cmd = instance.currentFrame.get().commandBuffer;
			if (indexBufferIndex - lastIndexBufferIndex != 0) {
				assert(indexBufferIndex <= indexBufferSize);

				cmd.drawIndexed(indexBufferIndex - lastIndexBufferIndex, 1, lastIndexBufferIndex, 0, 0);
			}

			if (early) {
				lastVertexBufferIndex = vertexBufferIndex;
				lastIndexBufferIndex = indexBufferIndex;
			} else {
				lastVertexBufferIndex = 0;
				vertexBufferIndex = 0;
				vertexArrIndex++;
				if (vertexArrIndex == vertexBuffers.size()) {
					vertexBuffers.emplace_back(std::make_unique<Buffer>(Buffer::Args{
						.size = sizeof(Vertex) * vertexBufferSize,
						.usage = vk::BufferUsageFlagBits::eVertexBuffer,
					}));
				}
				cmd.bindVertexBuffers(0, *vertexBuffers.at(vertexArrIndex)->buffer, {0});

				lastIndexBufferIndex = 0;
				indexBufferIndex = 0;
				indexArrIndex++;
				if (indexArrIndex == indexBuffers.size()) {
					indexBuffers.emplace_back(std::make_unique<Buffer>(Buffer::Args{
						.size = sizeof(uint16_t) * indexBufferSize,
						.usage = vk::BufferUsageFlagBits::eIndexBuffer,
					}));
				}
				cmd.bindIndexBuffer(*indexBuffers.at(indexArrIndex)->buffer, 0, vk::IndexType::eUint16);
			}
		}

	private:
		const size_t vertexBufferSize;
		const size_t indexBufferSize;

		Instance &instance;

		std::tuple<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
			vk::BufferCreateInfo bufferInfo{
				.size = size,
				.usage = usage,
				.sharingMode = vk::SharingMode::eExclusive,
			};

			auto buffer = Vulkan::device().resource.createBuffer(bufferInfo);

			vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo{
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
			};

			auto bufferMemory = Vulkan::device().resource.allocateMemory(allocInfo);

			buffer.bindMemory(*bufferMemory, 0);

			return {std::move(buffer), std::move(bufferMemory)};
		}

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
			auto memProperties = Vulkan::physicalDevice().getMemoryProperties();

			for (auto i: std::views::iota(0u, memProperties.memoryTypeCount)) {
				if (typeFilter & (1u << i) && (memProperties.memoryTypes.at(i).propertyFlags & properties) == properties) {
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type!");
		}

		void copyBuffer(const vk::Buffer &srcBuffer, const vk::Buffer &dstBuffer, vk::DeviceSize size, vk::DeviceSize offset = 0) {
			vk::CommandBufferAllocateInfo allocInfo{
				.commandPool = *instance.currentFrame.get().commandPool,
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = 1,
			};

			auto cmdBuffers = Vulkan::device().resource.allocateCommandBuffers(allocInfo);
			auto &cmdBuffer = cmdBuffers.front();

			vk::CommandBufferBeginInfo beginInfo{
				.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
			};

			cmdBuffer.begin(beginInfo);

			vk::BufferCopy copyRegion{
				.srcOffset = offset,
				.dstOffset = offset,
				.size = size,
			};
			cmdBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

			cmdBuffer.end();

			vk::SubmitInfo submitInfo{
				.commandBufferCount = 1,
				.pCommandBuffers = &*cmdBuffer,
			};

			auto graphicsQueue = Vulkan::getGraphicsQueue();

			graphicsQueue.resource.submit(submitInfo);
			graphicsQueue.resource.waitIdle();
		}
	};
}// namespace Engine