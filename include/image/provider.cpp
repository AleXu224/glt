#include "provider.hpp"

squi::ImageProvider squi::ImageProvider::fromFile(const std::string &path) {
	return squi::ImageProvider{
		.key = path,
		.provider = [path] {
			return ImageData::fromFile(path);
		},
	};
}

squi::ImageProvider squi::ImageProvider::fromUrl(const std::string &url) {
	return squi::ImageProvider{
		.key = url,
		.provider = [url] {
			try {
				return squi::ImageData::fromUrl(url);
			} catch (std::runtime_error e) {
				std::println("{}", e.what());
				return squi::ImageData{
					.data{0, 0, 0, 0},
					.width = 1,
					.height = 1,
					.channels = 4,
				};
			}
		},
	};
}
