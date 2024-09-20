#pragma once

#include "filesystem"
#include "functional"
#include "string"
#include "vector"


#include "skyr/v1/url.hpp"

namespace squi {
	struct FontProvider {
		std::string key;
		std::function<std::vector<char>(void)> provider;

		[[nodiscard]] static FontProvider fromFile(const std::filesystem::path &path);
		[[nodiscard]] static FontProvider fromUrl(const skyr::url &url);
	};
}// namespace squi