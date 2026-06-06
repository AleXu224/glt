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
#include <span>
#include <tuple>
#include <utility>
#include <vector>
#include <vulkan/vulkan_enums.hpp>


namespace glt::Engine {
	template<class Vertex, bool hasTexture = false, class... Uniforms>
	struct Pipeline : public std::enable_shared_from_this<Pipeline<Vertex, hasTexture, Uniforms...>> {
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

		struct PerFrameBuffers {
			size_t lastVertexBufferIndex = 0;
			size_t vertexBufferIndex = 0;
			size_t vertexArrIndex = 0;

			size_t lastIndexBufferIndex = 0;
			size_t indexBufferIndex = 0;
			size_t indexArrIndex = 0;

			uint32_t binds = 0;
			uint32_t transformIndex = 0;
			void const *lastBoundSampler = nullptr;

			std::vector<std::unique_ptr<Buffer>> vertexBuffers{};
			std::vector<std::unique_ptr<Buffer>> indexBuffers{};

			[[nodiscard]] Buffer &getCurrentVertexBuffer() {
				return *vertexBuffers.at(vertexArrIndex);
			}
			[[nodiscard]] Buffer &getCurrentIndexBuffer() {
				return *indexBuffers.at(indexArrIndex);
			}

			[[nodiscard]] std::tuple<uint64_t, uint64_t> availableSpace(size_t vertexBufferSize, size_t indexBufferSize) const {
				return {
					static_cast<uint64_t>(vertexBufferSize) - static_cast<uint64_t>(vertexBufferIndex),
					static_cast<uint64_t>(indexBufferSize) - static_cast<uint64_t>(indexBufferIndex),
				};
			}

			[[nodiscard]] std::pair<size_t, size_t> getIndexes(size_t vertexBufferSize, size_t indexBufferSize) const {
				return {vertexBufferIndex % vertexBufferSize, indexBufferIndex % indexBufferSize};
			}
		};

		std::vector<PerFrameBuffers> perFrame{};

		[[nodiscard]] PerFrameBuffers &currentFrameState() {
			return perFrame[instance.currentFrame.get().index];
		}
		[[nodiscard]] const PerFrameBuffers &currentFrameState() const {
			return perFrame[instance.currentFrame.get().index];
		}

		Pipeline(const Pipeline &) = delete;
		Pipeline(Pipeline &) = delete;
		Pipeline &operator=(const Pipeline &) = delete;
		Pipeline(const Pipeline &&) = delete;
		Pipeline(Pipeline &&) = delete;
		Pipeline &operator=(Pipeline &&) = delete;
		~Pipeline() = default;

		Pipeline(const Args &args)
			: fragmentShader(Vulkan::device(), args.fragmentShader),
			  vertexShader(Vulkan::device(), args.vertexShader),
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

				  auto &data = basicUniform.getData();
				  data.view = glm::mat4{
					  1.0f / (width / 2.f), 0.0f, 0.0f, 0.0f,
					  0.0f, 1.0f / (height / 2.f), 0.0f, 0.0f,
					  -1.0f, -1.0f, 1.0f, 0.0f,
					  0.0f, 0.0f, 0.0f, 1.0f
				  };

				  auto &state = currentFrameState();
				  state.vertexArrIndex = 0;
				  state.indexArrIndex = 0;
			  })),
			  frameEndListener(args.instance.frameEndEvent.observe([this] {
				  auto &state = currentFrameState();
				  state.lastVertexBufferIndex = 0;
				  state.vertexBufferIndex = 0;

				  state.lastIndexBufferIndex = 0;
				  state.indexBufferIndex = 0;

				  state.binds = 0;
			  })),
			  vertexBufferSize(args.vertexBufferSize),
			  indexBufferSize(args.IndexBufferSize),
			  instance(args.instance) {
			perFrame.resize(instance.frames.size());

			for (auto &f: perFrame) {
				f.vertexBuffers.emplace_back(std::make_unique<Buffer>(Buffer::Args{
					.size = sizeof(Vertex) * args.vertexBufferSize,
					.usage = vk::BufferUsageFlagBits::eVertexBuffer,
				}));
				f.indexBuffers.emplace_back(std::make_unique<Buffer>(Buffer::Args{
					.size = sizeof(uint16_t) * args.IndexBufferSize,
					.usage = vk::BufferUsageFlagBits::eIndexBuffer,
				}));
			}
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

			auto pushConstantRange = vk::PushConstantRange{
				.stageFlags = vk::ShaderStageFlagBits::eVertex,
				.offset = 0,
				.size = sizeof(PushConstant),
			};

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
				.setLayoutCount = setLayouts.size(),
				.pSetLayouts = setLayouts.data(),
				.pushConstantRangeCount = 1,
				.pPushConstantRanges = {&pushConstantRange},
			};

			layout = Vulkan::device().createPipelineLayout(pipelineLayoutInfo);

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

			pipeline = Vulkan::device().createGraphicsPipeline(nullptr, pipelineInfo);
		}

		std::function<void()> currentPipelineFlush = [&] {
			this->flush(true);
		};

		void bind() {
			auto &state = currentFrameState();
			auto isPipelineBound = instance.currentPipeline == this;
			auto isTransformBound = instance.getTransformIndex() == state.transformIndex;
			if (isPipelineBound && isTransformBound) return;
			if (instance.currentPipelineFlush) (*instance.currentPipelineFlush)();

			auto &cmd = instance.currentFrame.get().commandBuffer;
			if (!isPipelineBound) {
				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
				cmd.bindVertexBuffers(0, *state.vertexBuffers.at(state.vertexArrIndex)->buffer, {0});
				cmd.bindIndexBuffer(*state.indexBuffers.at(state.indexArrIndex)->buffer, 0, vk::IndexType::eUint16);
			}

			// In either case we'll need to bind the descriptors so no need to check
			auto descriptors = std::apply(
				[&](auto &&...U) {
					return std::array<vk::DescriptorSet, sizeof...(Uniforms) + 1>{basicUniform.getDescriptorSet(), (U.getDescriptorSet())...};
				},
				uniforms
			);

			cmd.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				*layout,
				0,
				descriptors,
				{}
			);

			state.transformIndex = instance.getTransformIndex();

			instance.currentPipeline = this;
			instance.currentPipelineFlush = &currentPipelineFlush;
			if (state.binds == 0) {
				instance.currentFrame.get().resourceLock.emplace_back(this->shared_from_this());
			}
			state.binds++;
		}

		void bindWithSampler(const SamplerUniform &sampler) {
			auto &state = currentFrameState();
			auto &cmd = instance.currentFrame.get().commandBuffer;
			if (instance.currentPipeline != this || state.lastBoundSampler != &sampler || instance.getTransformIndex() != state.transformIndex) {
				if (instance.currentPipelineFlush) (*instance.currentPipelineFlush)();
				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
				cmd.bindVertexBuffers(0, *state.vertexBuffers.at(state.vertexArrIndex)->buffer, {0});
				cmd.bindIndexBuffer(*state.indexBuffers.at(state.indexArrIndex)->buffer, 0, vk::IndexType::eUint16);

				auto descriptors = std::apply(
					[&](auto &&...U) {
						return std::array<vk::DescriptorSet, sizeof...(Uniforms) + 2>{basicUniform.getDescriptorSet(), sampler.getDescriptorSet(), (U.getDescriptorSet())...};
					},
					uniforms
				);

				cmd.bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					*layout,
					0,
					descriptors,
					{}
				);

				state.transformIndex = instance.getTransformIndex();

				state.lastBoundSampler = &sampler;
				if (state.binds == 0) {
					instance.currentFrame.get().resourceLock.emplace_back(this->shared_from_this());
				}
				instance.currentFrame.get().resourceLock.emplace_back(sampler.shared_from_this());
				state.binds++;
			}
			instance.currentPipeline = this;
			instance.currentPipelineFlush = &currentPipelineFlush;
		}

		[[nodiscard]] std::tuple<uint64_t, uint64_t> availableSpace() const {
			return currentFrameState().availableSpace(vertexBufferSize, indexBufferSize);
		}

		[[nodiscard]] std::pair<size_t, size_t> getIndexes() const {
			return currentFrameState().getIndexes(vertexBufferSize, indexBufferSize);
		}

		struct Data {
			const std::span<const Vertex> vertexes;
			const std::span<const uint16_t> indexes;
		};

		void addData(const Data &data) {
			auto &state = currentFrameState();
			assert(data.vertexes.size() < vertexBufferSize);
			assert(data.indexes.size() < indexBufferSize);

			auto [vertexSpace, indexSpace] = state.availableSpace(vertexBufferSize, indexBufferSize);

			size_t vertexOffset = 0;

			if (data.vertexes.size() >= vertexSpace || data.indexes.size() >= indexSpace) {
				vertexOffset = state.vertexBufferIndex;

				flush(false);
			}

			auto &vertexBuffer = state.getCurrentVertexBuffer();
			auto &indexBuffer = state.getCurrentIndexBuffer();

			memcpy((Vertex *) vertexBuffer.mappedMemory + state.vertexBufferIndex, data.vertexes.data(), data.vertexes.size() * sizeof(Vertex));
			memcpy((uint16_t *) indexBuffer.mappedMemory + state.indexBufferIndex, data.indexes.data(), data.indexes.size() * sizeof(uint16_t));
			if (vertexOffset) {
				for (size_t i = 0; i < data.indexes.size(); i++) {
					*((uint16_t *) indexBuffer.mappedMemory + i) -= vertexOffset;
				}
			}

			state.vertexBufferIndex += data.vertexes.size();
			state.indexBufferIndex += data.indexes.size();
			assert(state.indexBufferIndex <= indexBufferSize);
			assert(state.vertexBufferIndex <= vertexBufferSize);
		}

		void flush(bool early) {
			auto &state = currentFrameState();
			auto &cmd = instance.currentFrame.get().commandBuffer;
			if (state.indexBufferIndex - state.lastIndexBufferIndex != 0) {
				assert(state.indexBufferIndex <= indexBufferSize);

				PushConstant pushConstant{
					.model = instance.getTransform(),
				};

				cmd.pushConstants<PushConstant>(*layout, vk::ShaderStageFlagBits::eVertex, 0, pushConstant);
				cmd.drawIndexed(state.indexBufferIndex - state.lastIndexBufferIndex, 1, state.lastIndexBufferIndex, 0, 0);
			}

			if (early) {
				state.lastVertexBufferIndex = state.vertexBufferIndex;
				state.lastIndexBufferIndex = state.indexBufferIndex;
			} else {
				state.lastVertexBufferIndex = 0;
				state.vertexBufferIndex = 0;
				state.vertexArrIndex++;
				if (state.vertexArrIndex == state.vertexBuffers.size()) {
					state.vertexBuffers.emplace_back(std::make_unique<Buffer>(Buffer::Args{
						.size = sizeof(Vertex) * vertexBufferSize,
						.usage = vk::BufferUsageFlagBits::eVertexBuffer,
					}));
				}
				cmd.bindVertexBuffers(0, *state.vertexBuffers.at(state.vertexArrIndex)->buffer, {0});

				state.lastIndexBufferIndex = 0;
				state.indexBufferIndex = 0;
				state.indexArrIndex++;
				if (state.indexArrIndex == state.indexBuffers.size()) {
					state.indexBuffers.emplace_back(std::make_unique<Buffer>(Buffer::Args{
						.size = sizeof(uint16_t) * indexBufferSize,
						.usage = vk::BufferUsageFlagBits::eIndexBuffer,
					}));
				}
				cmd.bindIndexBuffer(*state.indexBuffers.at(state.indexArrIndex)->buffer, 0, vk::IndexType::eUint16);
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

			auto buffer = Vulkan::device().createBuffer(bufferInfo);

			vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

			vk::MemoryAllocateInfo allocInfo{
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
			};

			auto bufferMemory = Vulkan::device().allocateMemory(allocInfo);

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

			auto cmdBuffers = Vulkan::device().allocateCommandBuffers(allocInfo);
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
}// namespace glt::Engine