#pragma once
#include "getterSetter.hpp"
#include "pipeline.hpp"
#include "vulkanIncludes.hpp"
#include <array>
#include <cstddef>


namespace Engine {
	struct TextQuad {
		struct Vertex {
			alignas(16) glm::vec4 color;
			alignas(8) glm::vec2 size;
			alignas(8) glm::vec2 pos;
			alignas(8) glm::vec2 offset;
			alignas(8) glm::vec2 uv;
			alignas(8) glm::vec2 textUv;

			static std::array<vk::VertexInputAttributeDescription, 6> describe() {
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
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, size),
					},
					Desc{
						.location = 2,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, pos),
					},
					Desc{
						.location = 3,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, offset),
					},
					Desc{
						.location = 4,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, uv),
					},
					Desc{
						.location = 5,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, textUv),
					},
				};
			}
		};


		struct Args {
			glm::vec4 color = {1, 1, 1, 1};
			glm::vec2 position = {0, 0};
			glm::vec2 size = {0, 0};
			glm::vec2 offset = {0, 0};
			glm::vec2 uvTopLeft = {0, 0};
			glm::vec2 uvBottomRight = {0, 0};
		};

	private:
		std::array<Vertex, 4> vertices{};
		std::array<uint16_t, 6> indices{};

	public:
		void setPos(const squi::vec2 &newPos) {
			vertices[0].pos = newPos;
			vertices[1].pos = newPos;
			vertices[2].pos = newPos;
			vertices[3].pos = newPos;
		}
		squi::vec2 getPos() const {
			return vertices[0].pos;
		}
		squi::vec2 getSize() const {
			return vertices[0].size;
		}
		squi::vec2 getOffset() const {
			return vertices[0].offset;
		}

		TextQuad(const Args &args) {
			vertices[0] = {
				.color = args.color,
				.size = args.size,
				.pos = args.position,
				.offset = args.offset,
				.uv = {0, 0},
				.textUv = {args.uvTopLeft.x, args.uvTopLeft.y},
			};
			vertices[1] = {
				.color = args.color,
				.size = args.size,
				.pos = args.position,
				.offset = args.offset,
				.uv = {1, 0},
				.textUv = {args.uvBottomRight.x, args.uvTopLeft.y},
			};
			vertices[2] = {
				.color = args.color,
				.size = args.size,
				.pos = args.position,
				.offset = args.offset,
				.uv = {1, 1},
				.textUv = {args.uvBottomRight.x, args.uvBottomRight.y},
			};
			vertices[3] = {
				.color = args.color,
				.size = args.size,
				.pos = args.position,
				.offset = args.offset,
				.uv = {0, 1},
				.textUv = {args.uvTopLeft.x, args.uvBottomRight.y},
			};
		}

		Pipeline<Vertex, true>::Data getData(size_t vi, size_t ii) {
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