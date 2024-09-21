#pragma once

#include "functional"
#include "string"
#include "vector"

namespace squi {
	struct FontProvider {
		std::string key;
		std::function<std::vector<char>(void)> provider;

		[[nodiscard]] static FontProvider fromFile(const std::string &path);
		[[nodiscard]] static FontProvider fromUrl(const std::string &url);
	};
}// namespace squi