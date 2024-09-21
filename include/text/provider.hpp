#pragma once

#include "functional"
#include "string"
#include "vector"

namespace squi {
	struct FontProvider {
		std::string key;
		std::function<std::vector<char>(void)> provider;

		[[nodiscard]] static FontProvider fromFile(std::string_view path);
		[[nodiscard]] static FontProvider fromUrl(std::string_view url);
	};
}// namespace squi