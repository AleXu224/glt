#include "networking.hpp"
#include "image.hpp"
#include "renderer.hpp"
#include "vertex.hpp"
#include "widget.hpp"
#include <memory>
#include <string_view>
#include <thread>
#include "fstream"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image_ext.h"
#include "format"
#include "stdexcept"

using namespace squi;

Image::Data::Data(unsigned char *bytes, uint32_t length) {
	int width, height, channels;
	stbi_uc *data = stbi_load_from_memory(bytes, (int)length, &width, &height, &channels, 0);
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
		throw std::runtime_error(std::format("Unsupported number of channels: {}", channels));
	}
	this->width = width;
	this->height = height;
	this->channels = channels;
	this->ready = true;
	stbi_image_free(data);
}

Image::Data Image::Data::fromUrl(std::string_view url) {
	auto response = Networking::get(url);
	if (!response.success) {
		throw std::runtime_error(std::format("Failed to load image: {}", response.error));
	}
	return {reinterpret_cast<unsigned char *>(response.body.data()), (uint32_t)response.body.size()};
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

	return {bytes.data(), (uint32_t)length};
}

Image::Data Image::Data::fromUrlAsync(std::string_view url) {
	Data data{};

	data.loaderData = std::make_shared<Data>();
	std::thread([url, loaderData = data.loaderData](){
		*loaderData = Data::fromUrl(url);
		loaderData->ready = true;
	}).detach();

	return data;
}

Texture::Impl Image::Data::createTexture() const {
	if (!this->ready) {
		constexpr uint8_t data[4] = {1, 1, 1, 1};
		return {Texture{
			.width = 1,
			.height = 1,
			.channels = 4,
			.data = data,
		}};
	}
	return {Texture{
		.width = static_cast<uint16_t>(this->width),
		.height = static_cast<uint16_t>(this->height),
		.channels = static_cast<uint16_t>(this->channels),
		.data = this->data.data(),
		.dynamic = false,
	}};
}

Image::Impl::Impl(const Image &args)
	: Widget(args.widget, Widget::Flags::Default()),
	  texture(args.image.createTexture()),
	  fit(args.fit),
	  aspectRatio(static_cast<float>(args.image.width) / static_cast<float>(args.image.height)),
      width(args.image.width),
      height(args.image.height),
      quad(Quad::Args{
        .size = {0, 0},
        .texture = this->texture.getTextureView(),
        .textureType = TextureType::Texture,
      }) {

	if (!args.image.ready) {
		this->tempData = args.image.loaderData; 
	}
}

void Image::Impl::onUpdate() {
	if (!tempData) return;
	if (tempData->ready) {
		texture = tempData->createTexture();
		width = tempData->width;
		height = tempData->height;
		aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		quad = Quad::Args{
			.size = {0, 0},
			.texture = this->texture.getTextureView(),
			.textureType = TextureType::Texture,
		};
		tempData.reset();
	}
}

void Image::Impl::onLayout(vec2 &maxSize, vec2 &minSize) {
	switch (this->fit) {
		case Fit::none: {
			maxSize = {static_cast<float>(this->width), static_cast<float>(this->height)};
			minSize = {static_cast<float>(this->width), static_cast<float>(this->height)};
			break;
		}
		case Fit::fill: {
			break;
		}
		case Fit::cover: {
			if (maxSize.x / maxSize.y > this->aspectRatio) {
				maxSize.x = maxSize.y * this->aspectRatio;
			} else {
				maxSize.y = maxSize.x / this->aspectRatio;
			}
			break;
		}
		case Fit::contain: {
			if (maxSize.x / maxSize.y > this->aspectRatio) {
				maxSize.y = maxSize.x / this->aspectRatio;
			} else {
				maxSize.x = maxSize.y * this->aspectRatio;
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
