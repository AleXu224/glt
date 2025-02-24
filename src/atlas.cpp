#include "atlas.hpp"

#include <cstddef>

#include "store/texture.hpp"
#include "texture.hpp"

using namespace squi;

Atlas::Atlas(std::string_view key) : key(key), texture(squi::Store::Texture::getTexture(getProvier())) {}

std::tuple<vec2, vec2, bool> Atlas::add(const uint16_t &width, const uint16_t &height, unsigned char *data) {
	Atlas::Row *usedRow = nullptr;
	for (auto &row: rows) {
		if (row.availableWidth >= width) {
			if (row.height < height) {
				if (row.canBeMadeTaller && availableHeight >= static_cast<size_t>(height - row.height)) {
					availableHeight -= static_cast<size_t>(height - row.height);
					row.height = height;
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

		Atlas::Row newRow{
			.yOffset = static_cast<uint16_t>(AtlasSize - availableHeight),
			.height = static_cast<uint16_t>(height),
			.availableWidth = static_cast<uint16_t>(AtlasSize),
			.canBeMadeTaller = true
		};
		rows.emplace_back(std::move(newRow));

		usedRow = &rows.back();
		availableHeight -= height;
	}

	usedRow->elements.emplace_back(Atlas::Element{
		.xOffset = static_cast<uint16_t>(AtlasSize - usedRow->availableWidth),
		.width = static_cast<uint16_t>(width),
		.height = static_cast<uint16_t>(height),
	});

	// Copy data to atlas
	auto layout = texture->image.getSubresourceLayout(vk::ImageSubresource{
		.aspectMask = vk::ImageAspectFlagBits::eColor,
		.mipLevel = 0,
		.arrayLayer = 0,
	});
	for (int y = 0; y < height; y++) {
		auto *textureData = reinterpret_cast<unsigned char *>(this->texture->mappedMemory);
		auto yOffset = static_cast<ptrdiff_t>((y * layout.rowPitch) + usedRow->yOffset * layout.rowPitch);
		auto xOffset = static_cast<ptrdiff_t>(AtlasSize - usedRow->availableWidth);
		memcpy(textureData + yOffset + xOffset, data + static_cast<ptrdiff_t>(y * width), width);
	}

	// Prepare return values
	vec2 uvTopLeft{
		static_cast<float>(AtlasSize - usedRow->availableWidth) / static_cast<float>(AtlasSize),
		static_cast<float>(usedRow->yOffset) / static_cast<float>(AtlasSize)
	};
	vec2 uvBottomRight{
		static_cast<float>(AtlasSize - usedRow->availableWidth + width) / static_cast<float>(AtlasSize),
		static_cast<float>(usedRow->yOffset + height) / static_cast<float>(AtlasSize)
	};

	usedRow->availableWidth -= width;

	return {uvTopLeft, uvBottomRight, true};
}

ImageProvider squi::Atlas::getProvier() {
	return ImageProvider{
		.key = key,
		.provider = [&]() -> ImageData {
			return ImageData{
				.data = std::vector<unsigned char>(AtlasSize * AtlasSize, 0),
				.width = AtlasSize,
				.height = AtlasSize,
				.channels = 1,
			};
		},
	};
}

std::shared_ptr<Engine::Texture> squi::Atlas::getTexture() const {
	return texture;
}
