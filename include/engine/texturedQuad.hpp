#pragma once
#include "pipeline.hpp"
#include "vulkanIncludes.hpp"
#include <array>
#include <cstddef>
#include "getterSetter.hpp"


namespace Engine {
	struct TexturedQuad {
		struct Vertex {
			alignas(8) glm::vec2 size;
			alignas(8) glm::vec2 pos;
			alignas(8) glm::vec2 uv;

			static std::array<vk::VertexInputAttributeDescription, 3> describe() {
				using Desc = vk::VertexInputAttributeDescription;
				return {
					Desc{
						.location = 0,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, size),
					},
					Desc{
						.location = 1,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, pos),
					},
					Desc{
						.location = 2,
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

		TexturedQuad(const Args &args) {
			vertices[0] = {
				.size = args.size,
				.pos = args.position,
				.uv = {0, 0},
			};
			vertices[1] = {
				.size = args.size,
				.pos = args.position,
				.uv = {1, 0},
			};
			vertices[2] = {
				.size = args.size,
				.pos = args.position,
				.uv = {1, 1},
			};
			vertices[3] = {
				.size = args.size,
				.pos = args.position,
				.uv = {0, 1},
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