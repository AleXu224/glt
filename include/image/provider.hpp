#pragma once

#include "data.hpp"

#include "filesystem"
#include "skyr/v1/url.hpp"

namespace squi {
	struct ImageProvider {
		std::string key;
		std::function<ImageData(void)> provider;

		[[nodiscard]] static inline ImageProvider fromFile(const std::filesystem::path &path) {
			return ImageProvider{
				.key = path.string(),
				.provider = [path] {
					return ImageData::fromFile(path);
				},
			};
		}
		[[nodiscard]] static inline ImageProvider fromUrl(const skyr::url &url) {
			return ImageProvider{
				.key = url.href(),
				.provider = [url] {
					return ImageData::fromUrl(url);
				},
			};
		}
	};
}// namespace squi