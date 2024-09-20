#include "provider.hpp"

squi::ImageProvider squi::ImageProvider::fromFile(const std::filesystem::path &path) {
	return squi::ImageProvider{
		.key = path.string(),
		.provider = [path] {
			return ImageData::fromFile(path);
		},
	};
}

squi::ImageProvider squi::ImageProvider::fromUrl(const skyr::url &url) {
	return squi::ImageProvider{
		.key = url.href(),
		.provider = [url] {
			return squi::ImageData::fromUrl(url);
		},
	};
}
