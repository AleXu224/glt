#include "atlas.hpp"
#include "samplerUniform.hpp"
#include <array>
#include <cstddef>

using namespace squi;

Atlas::Atlas(Engine::Instance & instance) : sampler(Engine::SamplerUniform::Args{
	.instance = instance,
	.textureArgs{
		.instance = instance,
		.width = AtlasSize,
		.height = AtlasSize,
		.channels = 1,
	},
}){}

std::tuple<vec2, vec2, bool> Atlas::add(const uint16_t &width, const uint16_t &height, unsigned char *data) {
	AtlasRow *usedRow = nullptr;
	for (auto &row: rows) {
		if (row.availableWidth >= width) {
			if (row.height < height) {
				if (row.canBeMadeTaller && availableHeight >= height - row.height) {
					row.height = height;
					availableHeight -= height - row.height;
				} else {
					continue;
				}
			}
		} else {
			continue;
		}

		usedRow = &row;
	}
	if (usedRow == nullptr) {
		if (availableHeight < height) return {vec2(0, 0), vec2(0, 0), false};
		// Set last row to be not able to be made taller
		if (!rows.empty()) rows.back().canBeMadeTaller = false;

		AtlasRow newRow{
			.yOffset = static_cast<uint16_t>(AtlasSize - availableHeight),
			.height = static_cast<uint16_t>(height),
			.availableWidth = static_cast<uint16_t>(AtlasSize),
			.canBeMadeTaller = true};
		rows.emplace_back(std::move(newRow));

		usedRow = &rows.back();
		availableHeight -= height;
	}

	usedRow->elements.emplace_back(AtlasElement{
		.xOffset = static_cast<uint16_t>(AtlasSize - usedRow->availableWidth),
		.width = static_cast<uint16_t>(width),
		.height = static_cast<uint16_t>(height)});

	// Copy data to atlas
	for (int y = 0; y < height; y++) {
		memcpy(&getAtlasData().at((y + usedRow->yOffset) * AtlasSize + AtlasSize - usedRow->availableWidth), data + static_cast<ptrdiff_t>(y * width), width);
	}

	// Prepare return values
	vec2 uvTopLeft{
		static_cast<float>(AtlasSize - usedRow->availableWidth) / static_cast<float>(AtlasSize),
		static_cast<float>(usedRow->yOffset) / static_cast<float>(AtlasSize)};
	vec2 uvBottomRight{
		static_cast<float>(AtlasSize - usedRow->availableWidth + width) / static_cast<float>(AtlasSize),
		static_cast<float>(usedRow->yOffset + height) / static_cast<float>(AtlasSize)};

	usedRow->availableWidth -= width;

	return {uvTopLeft, uvBottomRight, true};
}

std::array<unsigned char, AtlasSize * AtlasSize> &Atlas::getAtlasData() const {
	return *reinterpret_cast<std::array<unsigned char, AtlasSize * AtlasSize>*>(sampler.texture.mappedMemory);
}