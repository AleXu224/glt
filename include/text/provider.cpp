#include "provider.hpp"

#include "filesystem"
#include "fstream"
#include "networking.hpp"


squi::FontProvider squi::FontProvider::fromFile(std::string_view path) {
	return squi::FontProvider{
		.key = std::string{path},
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

squi::FontProvider squi::FontProvider::fromUrl(std::string_view url) {
	return FontProvider{
		.key = std::string{url},
		.provider = [url] {
			auto data = Networking::get(url);
			if (!data.success) return std::vector<char>{};

			return std::vector<char>(data.body.begin(), data.body.end());
		},
	};
}
