#include "data.hpp"

#include "filesystem"
#include "fstream"
#include "print"

#include "loader.hpp"
#include "networking.hpp"

#include "engine/texture.hpp"

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

squi::ImageData ImageData::fromUrl(const std::string &url) {
	auto response = Networking::get(url);
	if (!response.success) {
		throw std::runtime_error(std::format("Failed to load image: {}", response.error));
	}
	return fromBytes(reinterpret_cast<unsigned char *>(response.body.data()), (uint32_t) response.body.size());
}

squi::ImageData ImageData::fromFile(const std::string &path) {
	auto getEmptyImage = []() {
		return squi::ImageData{.data = {0, 0, 0, 0}, .width = 1, .height = 1, .channels = 4};
	};

	auto s = std::ifstream(std::filesystem::path{path}, std::ios::binary);

	if (!s) {
		std::println("Failed to open file {}", path);
		return getEmptyImage();
	}

	std::stringstream bytes{};
	bytes << s.rdbuf();

	auto str = bytes.str();
	return ImageData::fromBytes(reinterpret_cast<unsigned char *>(str.data()), static_cast<uint32_t>(str.size()));
}

std::future<ImageData> ImageData::fromUrlAsync(const std::string &url) {
	return std::async(std::launch::async, [url]() {
		return ImageData::fromUrl(url);
	});
}

std::future<ImageData> ImageData::fromFileAsync(const std::string &path) {
	return std::async(std::launch::async, [path = path]() {
		return ImageData::fromFile(path);
	});
}

std::shared_ptr<Engine::Texture> ImageData::createTexture() const {
	auto texture = std::make_shared<Engine::Texture>(Engine::Texture::Args{
		.width = width,
		.height = height,
		.channels = channels,
		.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1,
	});
	// auto layout = texture->image.getSubresourceLayout(vk::ImageSubresource{
	// 	.aspectMask = vk::ImageAspectFlagBits::eColor,
	// 	.mipLevel = 0,
	// 	.arrayLayer = 0,
	// });
	auto writer = texture->getWriter();
	for (uint32_t row = 0; row < height; row++) {
		memcpy(
			reinterpret_cast<uint8_t *>(writer.memory) + row * width * channels,
			data.data() + static_cast<ptrdiff_t>(row * width * channels),
			static_cast<size_t>(width) * static_cast<size_t>(channels)
		);
	}
	writer.write();
	texture->generateMipmaps();
	return texture;
}