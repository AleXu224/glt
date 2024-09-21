#include "provider.hpp"

squi::ImageProvider squi::ImageProvider::fromFile(std::string_view path) {
	return squi::ImageProvider{
		.key = std::string{path},
		.provider = [path] {
			return ImageData::fromFile(path);
		},
	};
}

squi::ImageProvider squi::ImageProvider::fromUrl(std::string_view url) {
	return squi::ImageProvider{
		.key = std::string{url},
		.provider = [url] {
			return squi::ImageData::fromUrl(url);
		},
	};
}
