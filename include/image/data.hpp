#pragma once

#include "cstdint"
#include "future"
#include "vector"

namespace Engine {
	struct Texture;
}

namespace squi {
	struct ImageData {
		std::vector<uint8_t> data;
		uint32_t width;
		uint32_t height;
		uint32_t channels;

		static ImageData fromBytes(unsigned char *bytes, uint32_t length);
		static ImageData fromUrl(std::string_view url);
		static ImageData fromFile(std::string_view path);
		static std::future<ImageData> fromUrlAsync(std::string_view url);
		static std::future<ImageData> fromFileAsync(std::string_view path);

		[[nodiscard]] std::shared_ptr<Engine::Texture> createTexture() const;
	};
}// namespace squi