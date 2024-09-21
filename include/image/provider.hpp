#pragma once

#include "data.hpp"

namespace squi {
	struct ImageProvider {
		std::string key;
		std::function<ImageData(void)> provider;

		[[nodiscard]] static ImageProvider fromFile(std::string_view path);
		[[nodiscard]] static ImageProvider fromUrl(std::string_view url);
	};
}// namespace squi