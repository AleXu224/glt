#pragma once

#include "data.hpp"

namespace squi {
	struct ImageProvider {
		std::string key;
		std::function<ImageData(void)> provider;

		[[nodiscard]] static ImageProvider fromFile(const std::string &path);
		[[nodiscard]] static ImageProvider fromUrl(const std::string &url);
	};
}// namespace squi