#pragma once

#include "cstdint"
#include "filesystem"
#include "future"
#include "vector"

#include "engine/texture.hpp"

#include "skyr/v1/url.hpp"

namespace squi {
	struct ImageData {
		std::vector<uint8_t> data;
		uint32_t width;
		uint32_t height;
		uint32_t channels;

		static ImageData fromBytes(unsigned char *bytes, uint32_t length);
		static ImageData fromUrl(const skyr::url &url);
		static ImageData fromFile(const std::filesystem::path &path);
		static std::future<ImageData> fromUrlAsync(const skyr::url &url);
		static std::future<ImageData> fromFileAsync(const std::filesystem::path &path);

		[[nodiscard]] std::shared_ptr<Engine::Texture> createTexture() const;
	};
}// namespace squi