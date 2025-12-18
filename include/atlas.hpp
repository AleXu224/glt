#pragma once

#include "image/provider.hpp"
#include "texture.hpp"
#include "vec2.hpp"

#include "vector"

constexpr size_t AtlasSize = 1024;

namespace squi {

	class Atlas {
		struct Element {
			uint16_t xOffset = 0;
			uint16_t width = 0;
			uint16_t height = 0;
			std::vector<unsigned char> data{};
		};

		struct Row {
			std::vector<Element> elements{};
			uint16_t yOffset;
			uint16_t height;
			uint16_t availableWidth;
			bool canBeMadeTaller;
		};
		// Allocating on the heap because it's too big for the stack
		std::vector<Row> rows{};
		std::vector<unsigned char> shadowBuffer = std::vector<unsigned char>(AtlasSize * AtlasSize, 0);
		size_t availableHeight = AtlasSize;

		std::string key;

		std::shared_ptr<Engine::Texture> texture;
		std::optional<Engine::TextureWriter> textureWriter = std::nullopt;


	public:
		Atlas(std::string_view key);

		std::tuple<vec2 /*uvTopLeft*/, vec2 /*uvBottomRight*/, bool /*success*/>
		add(const uint16_t &width, const uint16_t &height, unsigned char *data);

		[[nodiscard]] std::shared_ptr<Engine::Texture> getTexture() const;
		[[nodiscard]] ImageProvider getProvier();

		void writePendingTextures();
	};
}// namespace squi