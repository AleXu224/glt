#include "data.hpp"

#include "fstream"
#include "print"

#include "loader.hpp"
#include "networking.hpp"


using namespace squi;

squi::ImageData ImageData::fromBytes(unsigned char *bytes, uint32_t length) {
	auto res = loadImageInto(bytes, length);

	return ImageData{
		.data = res.data,
		.width = res.width,
		.height = res.height,
		.channels = res.channels,
	};
}

squi::ImageData ImageData::fromUrl(const skyr::url &url) {
	auto response = Networking::get(url);
	if (!response.success) {
		throw std::runtime_error(std::format("Failed to load image: {}", response.error));
	}
	return fromBytes(reinterpret_cast<unsigned char *>(response.body.data()), (uint32_t) response.body.size());
}

squi::ImageData ImageData::fromFile(const std::filesystem::path &path) {
	auto getEmptyImage = []() {
		return squi::ImageData{{0, 0, 0, 0}, 1, 1, 4};
	};

	std::ifstream s{path, std::ios::binary};

	if (!s) {
		std::println("Failed to open file {}", path.string());
		return getEmptyImage();
	}

	std::stringstream bytes{};
	bytes << s.rdbuf();

	auto str = bytes.str();
	return ImageData::fromBytes(reinterpret_cast<unsigned char *>(str.data()), static_cast<uint32_t>(str.size()));
}

std::future<ImageData> ImageData::fromUrlAsync(const skyr::url &url) {
	return std::async(std::launch::async, [url]() {
		return ImageData::fromUrl(url);
	});
}

std::future<ImageData> ImageData::fromFileAsync(const std::filesystem::path &path) {
	return std::async(std::launch::async, [path = path]() {
		return ImageData::fromFile(path);
	});
}

std::shared_ptr<Engine::Texture> ImageData::createTexture() const {
	auto texture = std::make_shared<Engine::Texture>(Engine::Texture::Args{
		.width = width,
		.height = height,
		.channels = channels,
	});
	auto layout = texture->image.getSubresourceLayout(vk::ImageSubresource{
		.aspectMask = vk::ImageAspectFlagBits::eColor,
		.mipLevel = 0,
		.arrayLayer = 0,
	});
	for (uint32_t row = 0; row < height; row++) {
		memcpy(
			reinterpret_cast<uint8_t *>(texture->mappedMemory) + row * layout.rowPitch,
			data.data() + static_cast<ptrdiff_t>(row * width * channels),
			static_cast<size_t>(width) * static_cast<size_t>(channels)
		);
	}
	return texture;
}