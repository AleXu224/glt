#include "networking.hpp"

#include "image.hpp"
#include "fstream"
#include "renderer.hpp"
#include "vertex.hpp"
#include "widget.hpp"
#include <future>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image_ext.h"
#include "format"
#include "stdexcept"

using namespace squi;

Image::Data::Data(unsigned char *bytes, uint32_t length) {
	int width, height, channels;
	stbi_uc *data = stbi_load_from_memory(bytes, (int) length, &width, &height, &channels, 0);
	if (data == nullptr) {
		throw std::runtime_error("Failed to load image");
	}
	if (channels == 3) {
		channels = 4;
		this->data.resize(width * height * 4);
		for (int i = 0; i < width * height; i++) {
			this->data[i * 4 + 0] = data[i * 3 + 0];
			this->data[i * 4 + 1] = data[i * 3 + 1];
			this->data[i * 4 + 2] = data[i * 3 + 2];
			this->data[i * 4 + 3] = 255;
		}
	} else if (channels == 1 || channels == 2 || channels == 4) {
		this->data.resize(width * height * channels);
		std::memcpy(this->data.data(), data, this->data.size());
	} else {
		throw std::runtime_error("Unsupported number of channels");
	}
	this->width = width;
	this->height = height;
	this->channels = channels;
	stbi_image_free(data);
}

Image::Data Image::Data::fromUrl(std::string_view url) {
	auto response = Networking::get(url);
	if (!response.success) {
		throw std::runtime_error(std::format("Failed to load image: {}", response.error));
	}
	return {reinterpret_cast<unsigned char *>(response.body.data()), (uint32_t) response.body.size()};
}

Image::Data Image::Data::fromFile(std::string_view path) {
	std::fstream s{path.data(), std::ios::binary | std::ios::in};

	if (!s.is_open()) {
		throw std::runtime_error(std::format("Failed to load image: {}", path));
	}

	s.seekg(0, std::ios::end);
	auto length = s.tellg();
	s.seekg(0, std::ios::beg);

	std::vector<unsigned char> bytes(length);
	s.read(reinterpret_cast<char *>(bytes.data()), length);

	return {bytes.data(), (uint32_t) length};
}

std::future<Image::Data> Image::Data::fromUrlAsync(std::string_view url) {
	return std::async(std::launch::async, [url]() {
		return Data::fromUrl(url);
	});
}

std::future<Image::Data> Image::Data::fromFileAsync(std::string_view path) {
	return std::async(std::launch::async, [path]() {
		return Data::fromFile(path);
	});
}

Texture::Impl Image::Data::createTexture() const {
	return {Texture{
		.width = static_cast<uint16_t>(this->width),
		.height = static_cast<uint16_t>(this->height),
		.channels = static_cast<uint16_t>(this->channels),
		.data = this->data.data(),
		.dynamic = false,
	}};
}

struct ImageState {
	bool valid = true;
	bool ready = false;
};

Image::Impl::Impl(const Image &args)
	: Widget(args.widget, Widget::Flags::Default()),
	  texture(Texture::Empty()),
	  fit(args.fit),
	  quad(Quad::Args{
		  .size = {0, 0},
		  .texture = this->texture.getTextureView(),
		  .textureType = TextureType::Texture,
	  }) {

	ImageState imageState{};
	switch (args.image.index()) {
		case 0: {
			imageState.ready = false;
			auto data = std::get<0>(args.image);
			std::future<Texture::Impl> texFuture = std::async(std::launch::async, [data = data]() {
				return data.createTexture();
			});
			state.properties.insert({"imageFuture", texFuture.share()});
			break;
		}
		case 1: {
			auto &future = std::get<1>(args.image);
			imageState.ready = false;
			if (!future.valid()) {
#ifdef _DEBUG
				printf("Warning: Image future is not valid\n");
#endif
				imageState.valid = false;
			}
			std::future<Texture::Impl> texFuture = std::async(std::launch::async, [future = future]() {
				future.wait();
				return future.get().createTexture();
			});
			state.properties.insert({"imageFuture", texFuture.share()});
		}
	}
	state.properties.insert({"imageState", imageState});
}

void Image::Impl::onUpdate() {
	auto &imageState = std::any_cast<ImageState &>(state.properties.at("imageState"));
	if (imageState.ready || !imageState.valid) return;
	auto &future = std::any_cast<std::shared_future<Texture::Impl> &>(state.properties.at("imageFuture"));
	const auto status = future.wait_for(std::chrono::seconds(0));
	if (status == std::future_status::ready) {
		texture = future.get();
		quad = Quad::Args{
			.size = {0, 0},
			.texture = this->texture.getTextureView(),
			.textureType = TextureType::Texture,
		};
		imageState.ready = true;
		auto iter = state.properties.find("imageFuture");
		if (iter != state.properties.end()) {
			state.properties.erase(iter);
		}
	}
}

void Image::Impl::onLayout(vec2 &maxSize, vec2 &minSize) {
	const auto &properties = texture.getProperties();
	const float aspectRatio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
	switch (this->fit) {
		case Fit::none: {
			maxSize = {static_cast<float>(properties.width), static_cast<float>(properties.height)};
			// minSize = {static_cast<float>(this->width), static_cast<float>(this->height)};
			break;
		}
		case Fit::fill: {
			break;
		}
		case Fit::cover: {
			if (maxSize.x / maxSize.y > aspectRatio) {
				maxSize.x = maxSize.y * aspectRatio;
			} else {
				maxSize.y = maxSize.x / aspectRatio;
			}
			break;
		}
		case Fit::contain: {
			if (maxSize.x / maxSize.y > aspectRatio) {
				maxSize.y = maxSize.x / aspectRatio;
			} else {
				maxSize.x = maxSize.y * aspectRatio;
			}
			break;
		}
	}
}

void Image::Impl::postLayout(vec2 &size) {
	quad.setSize(getSize());
}

void Image::Impl::postArrange(vec2 &pos) {
	this->quad.setPos(pos);
}

void Image::Impl::onDraw() {
	Renderer::getInstance().addQuad(quad);
}
