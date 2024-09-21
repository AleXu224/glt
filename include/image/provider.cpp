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
			return squi::ImageData::fromUrl(url);
		},
	};
}
