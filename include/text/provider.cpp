#include "provider.hpp"

#include "filesystem"
#include "fstream"
#include "networking.hpp"


squi::FontProvider squi::FontProvider::fromFile(const std::string &path) {
	return squi::FontProvider{
		.key = path,
		.provider = [path = std::filesystem::path(path)] {
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

squi::FontProvider squi::FontProvider::fromUrl(const std::string &url) {
	return FontProvider{
		.key = url,
		.provider = [url] {
			auto data = Networking::get(url);
			if (!data.success) return std::vector<char>{};

			return std::vector<char>(data.body.begin(), data.body.end());
		},
	};
}
