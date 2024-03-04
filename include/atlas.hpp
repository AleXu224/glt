#pragma once

#include "array"
#include "engine/samplerUniform.hpp"
#include "vec2.hpp"
#include "vector"

constexpr size_t AtlasSize = 1024;

namespace squi {
	struct AtlasElement {
		uint16_t xOffset = 0;
		uint16_t width = 0;
		uint16_t height = 0;
	};

	struct AtlasRow {
		std::vector<AtlasElement> elements{};
		uint16_t yOffset;
		uint16_t height;
		uint16_t availableWidth;
		bool canBeMadeTaller;
	};

	class Atlas {
		// Allocating on the heap because it's too big for the stack
		std::vector<AtlasRow> rows{};
		size_t availableHeight = AtlasSize;

	public:
		Engine::SamplerUniform sampler;

		Atlas(Engine::Instance &instance);

		std::tuple<vec2 /*uvTopLeft*/, vec2 /*uvBottomRight*/, bool /*success*/>
		add(const uint16_t &width, const uint16_t &height, unsigned char *data);

		[[nodiscard]] std::array<unsigned char, AtlasSize * AtlasSize> &getAtlasData() const;
	};
}// namespace squi