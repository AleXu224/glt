#pragma once

#include "array"
#include "d3d11.h"
#include "memory"
#include "vec2.hpp"
#include "vector"

constexpr uint16_t ATLAS_SIZE = 1024;

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
		std::unique_ptr<std::array<unsigned char, ATLAS_SIZE * ATLAS_SIZE>> atlas = std::make_unique<std::array<unsigned char, ATLAS_SIZE * ATLAS_SIZE>>();
		std::vector<AtlasRow> rows{};
		uint16_t availableHeight{ATLAS_SIZE};
		std::shared_ptr<ID3D11Texture2D> texture{};
		bool textureDirty = false;

	public:
		std::shared_ptr<ID3D11ShaderResourceView> textureView{};

		Atlas();

		std::tuple<vec2 /*uvTopLeft*/, vec2 /*uvBottomRight*/, bool /*success*/>
		add(const uint16_t &width, const uint16_t &height, unsigned char *data);

		std::array<unsigned char, ATLAS_SIZE * ATLAS_SIZE> &getAtlasData();

		void updateTexture();
	};
}// namespace squi