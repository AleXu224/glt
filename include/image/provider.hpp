#pragma once

#include "data.hpp"

#include "filesystem"
#include "skyr/v1/url.hpp"

namespace squi {
	struct ImageProvider {
		std::string key;
		std::function<ImageData(void)> provider;

		[[nodiscard]] static ImageProvider fromFile(const std::filesystem::path &path);
		[[nodiscard]] static ImageProvider fromUrl(const skyr::url &url);
	};
}// namespace squi