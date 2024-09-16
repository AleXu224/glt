#pragma once

#include "filesystem"
#include "fstream"
#include "functional"
#include "vector"


#include "networking.hpp"

#include "skyr/v1/url.hpp"

namespace squi {
	struct FontProvider {
		std::string key;
		std::function<std::vector<char>(void)> provider;

		[[nodiscard]] static inline FontProvider fromFile(const std::filesystem::path &path) {
			return FontProvider{
				.key = path.string(),
				.provider = [path] {
					std::ifstream input(path, std::ifstream::binary);
					if (!input.is_open()) {
						return std::vector<char>{};
					}

					std::stringstream bytes{};
					bytes << input.rdbuf();
					auto str = bytes.str();

					return std::vector<char>(
						str.begin(), str.end()
					);
				},
			};
		}
		[[nodiscard]] static inline FontProvider fromUrl(const skyr::url &url) {
			return FontProvider{
				.key = url.href(),
				.provider = [url] {
					auto data = Networking::get(url);
					if (!data.success) return std::vector<char>{};

					return std::vector<char>(data.body.begin(), data.body.end());
				},
			};
		}
	};
}// namespace squi