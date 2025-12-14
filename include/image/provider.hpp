#pragma once

#include "data.hpp"

namespace squi {
	struct ImageProvider {
		std::string key;
		std::function<ImageData(void)> provider;

		[[nodiscard]] static ImageProvider fromFile(const std::string &path);
		[[nodiscard]] static ImageProvider fromUrl(const std::string &url);

		bool operator==(const ImageProvider &other) const {
			return key == other.key;
		}
	};
}// namespace squi