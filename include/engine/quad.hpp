#pragma once
#include "getterSetter.hpp"
#include "pipeline.hpp"
#include "vulkanIncludes.hpp"
#include <array>
#include <cstddef>


namespace Engine {
	struct Quad {
		struct Vertex {
			alignas(16) glm::vec4 color;
			alignas(16) glm::vec4 borderColor;
			alignas(16) glm::vec4 borderRadiuses;
			alignas(16) glm::vec4 borderSizes;
			alignas(8) glm::vec2 size;
			alignas(8) glm::vec2 pos;
			alignas(8) glm::vec2 uv;

			static std::array<vk::VertexInputAttributeDescription, 7> describe() {
				using Desc = vk::VertexInputAttributeDescription;
				return {
					Desc{
						.location = 0,
						.binding = 0,
						.format = vk::Format::eR32G32B32A32Sfloat,
						.offset = offsetof(Vertex, color),
					},
					Desc{
						.location = 1,
						.binding = 0,
						.format = vk::Format::eR32G32B32A32Sfloat,
						.offset = offsetof(Vertex, borderColor),
					},
					Desc{
						.location = 2,
						.binding = 0,
						.format = vk::Format::eR32G32B32A32Sfloat,
						.offset = offsetof(Vertex, borderRadiuses),
					},
					Desc{
						.location = 3,
						.binding = 0,
						.format = vk::Format::eR32G32B32A32Sfloat,
						.offset = offsetof(Vertex, borderSizes),
					},
					Desc{
						.location = 4,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, size),
					},
					Desc{
						.location = 5,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, pos),
					},
					Desc{
						.location = 6,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, uv),
					},
				};
			}
		};


		struct Args {
			glm::vec2 position = {0, 0};
			glm::vec2 size = {0, 0};
			glm::vec4 color = {1, 1, 1, 1};
			glm::vec4 borderRadiuses{0};
			glm::vec4 borderSizes{0};
			glm::vec4 borderColor{0};
		};

	private:
		std::array<Vertex, 4> vertices{};
		std::array<uint16_t, 6> indices{};

	public:
		GetterSetter<glm::vec2, glm::vec2, glm::vec2, glm::vec2> position{
			vertices[0].pos,
			vertices[1].pos,
			vertices[2].pos,
			vertices[3].pos,
		};
		GetterSetter<glm::vec2, glm::vec2, glm::vec2, glm::vec2> size{
			vertices[0].size,
			vertices[1].size,
			vertices[2].size,
			vertices[3].size,
		};
		GetterSetter<glm::vec4, glm::vec4, glm::vec4, glm::vec4> color{
			vertices[0].color,
			vertices[1].color,
			vertices[2].color,
			vertices[3].color,
		};
		GetterSetter<glm::vec4, glm::vec4, glm::vec4, glm::vec4> borderColor{
			vertices[0].borderColor,
			vertices[1].borderColor,
			vertices[2].borderColor,
			vertices[3].borderColor,
		};
		struct BorderRadiuses {
			GetterSetter<glm::vec4, glm::vec4, glm::vec4, glm::vec4> all;
			GetterSetter<float, float, float, float> topLeft;
			GetterSetter<float, float, float, float> topRight;
			GetterSetter<float, float, float, float> bottomRight;
			GetterSetter<float, float, float, float> bottomLeft;
		};
		BorderRadiuses borderRadiuses{
			.all{
				vertices[0].borderRadiuses,
				vertices[1].borderRadiuses,
				vertices[2].borderRadiuses,
				vertices[3].borderRadiuses,
			},
			.topLeft{
				vertices[0].borderRadiuses[0],
				vertices[1].borderRadiuses[0],
				vertices[2].borderRadiuses[0],
				vertices[3].borderRadiuses[0],
			},
			.topRight{
				vertices[0].borderRadiuses[1],
				vertices[1].borderRadiuses[1],
				vertices[2].borderRadiuses[1],
				vertices[3].borderRadiuses[1],
			},
			.bottomRight{
				vertices[0].borderRadiuses[2],
				vertices[1].borderRadiuses[2],
				vertices[2].borderRadiuses[2],
				vertices[3].borderRadiuses[2],
			},
			.bottomLeft{
				vertices[0].borderRadiuses[3],
				vertices[1].borderRadiuses[3],
				vertices[2].borderRadiuses[3],
				vertices[3].borderRadiuses[3],
			},
		};
		struct BorderSizes {
			GetterSetter<glm::vec4, glm::vec4, glm::vec4, glm::vec4> all;
			GetterSetter<float, float, float, float> top;
			GetterSetter<float, float, float, float> right;
			GetterSetter<float, float, float, float> bottom;
			GetterSetter<float, float, float, float> left;
		};
		BorderSizes borderSizes{
			.all{
				vertices[0].borderSizes,
				vertices[1].borderSizes,
				vertices[2].borderSizes,
				vertices[3].borderSizes,
			},
			.top{
				vertices[0].borderSizes[0],
				vertices[1].borderSizes[0],
				vertices[2].borderSizes[0],
				vertices[3].borderSizes[0],
			},
			.right{
				vertices[0].borderSizes[1],
				vertices[1].borderSizes[1],
				vertices[2].borderSizes[1],
				vertices[3].borderSizes[1],
			},
			.bottom{
				vertices[0].borderSizes[2],
				vertices[1].borderSizes[2],
				vertices[2].borderSizes[2],
				vertices[3].borderSizes[2],
			},
			.left{
				vertices[0].borderSizes[3],
				vertices[1].borderSizes[3],
				vertices[2].borderSizes[3],
				vertices[3].borderSizes[3],
			},
		};

		Quad(const Args &args) {
			vertices[0] = {
				.color = args.color,
				.borderColor = args.borderColor,
				.borderRadiuses = args.borderRadiuses,
				.borderSizes = args.borderSizes,
				.size = args.size,
				.pos = args.position,
				.uv = {0, 0},
			};
			vertices[1] = {
				.color = args.color,
				.borderColor = args.borderColor,
				.borderRadiuses = args.borderRadiuses,
				.borderSizes = args.borderSizes,
				.size = args.size,
				.pos = args.position,
				.uv = {1, 0},
			};
			vertices[2] = {
				.color = args.color,
				.borderColor = args.borderColor,
				.borderRadiuses = args.borderRadiuses,
				.borderSizes = args.borderSizes,
				.size = args.size,
				.pos = args.position,
				.uv = {1, 1},
			};
			vertices[3] = {
				.color = args.color,
				.borderColor = args.borderColor,
				.borderRadiuses = args.borderRadiuses,
				.borderSizes = args.borderSizes,
				.size = args.size,
				.pos = args.position,
				.uv = {0, 1},
			};
		}

		Pipeline<Vertex>::Data getData(size_t vi, size_t ii) {
			(void) vi;
			(void) ii;

			indices[0] = 0 + vi;
			indices[1] = 1 + vi;
			indices[2] = 2 + vi;
			indices[3] = 0 + vi;
			indices[4] = 2 + vi;
			indices[5] = 3 + vi;

			return {
				.vertexes = vertices,
				.indexes = indices,
			};
		}
	};
}// namespace Engine