#include "provider.hpp"

#include "fstream"
#include "networking.hpp"


squi::FontProvider squi::FontProvider::fromFile(const std::filesystem::path &path) {
	return squi::FontProvider{
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

squi::FontProvider squi::FontProvider::fromUrl(const skyr::url &url) {
	return FontProvider{
		.key = url.href(),
		.provider = [url] {
			auto data = Networking::get(url);
			if (!data.success) return std::vector<char>{};

			return std::vector<char>(data.body.begin(), data.body.end());
		},
	};
}
