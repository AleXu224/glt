#include "networking.hpp"

#include "fstream"
#include "image.hpp"
#include "renderer.hpp"
#include "vertex.hpp"
#include "widget.hpp"
#include <future>
#include <iostream>
#include <string_view>


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
		std::cout << "Failed to open file " << path << std::endl;
		return {{0, 0, 0, 0}, 1, 1, 4};
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
	std::string pathStr{path};
	return std::async(std::launch::async, [path = pathStr]() {
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

Image::Impl::Impl(const Image &args)
	: Widget(args.widget, Widget::Flags{
		.shouldLayoutChildren = false,
	}),
	  texture(Texture::Empty()),
	  fit(args.fit),
	  quad(Quad::Args{
		  .size = {0, 0},
		  .texture = this->texture.getTextureView(),
		  .textureType = TextureType::Texture,
	  }) {

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
}

void Image::Impl::onUpdate() {
	if (imageState.ready || !imageState.valid) return;
	auto &future = std::any_cast<std::shared_future<Texture::Impl> &>(state.properties.at("imageFuture"));
	// const auto status = future.wait_for(std::chrono::seconds(0));
	if (future._Is_ready()) {
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
		reLayout();
	}
}

void Image::Impl::onLayout(vec2 &maxSize, vec2 &minSize) {
	const auto &properties = texture.getProperties();
	const float aspectRatio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
	switch (this->fit) {
		case Fit::none: {
			maxSize.x = std::min(maxSize.x, static_cast<float>(properties.width));
			maxSize.y = std::min(maxSize.y, static_cast<float>(properties.height));
			break;
		}
		case Fit::fill: {
			break;
		}
		case Fit::cover: {
			break;
		}
		case Fit::contain: {
			if (maxSize.x / maxSize.y > aspectRatio) {
				maxSize.x = maxSize.y * aspectRatio;
			} else {
				maxSize.y = maxSize.x / aspectRatio;
			}
			break;
		}
	}
}

void Image::Impl::postLayout(vec2 &size) {
	const auto &properties = texture.getProperties();

	switch (this->fit) {
		case Fit::none: {
			quad.setSize(vec2{
				static_cast<float>(properties.width),
				static_cast<float>(properties.height),
			});
			break;
		}
		case Fit::fill:
		case Fit::contain: {
			quad.setSize(size);
			break;
		}
		case Fit::cover: {
			const float aspectRatio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
			if (size.x / size.y > aspectRatio) {
				quad.setSize(vec2{
					size.x,
					size.x / aspectRatio,
				});
			} else {
				quad.setSize(vec2{
					size.y * aspectRatio,
					size.y,
				});
			}
			break;
		}
	}
}

void Image::Impl::postArrange(vec2 &pos) {
	const auto &widgetSize = getSize();
	const vec2 quadSize = quad.getData().size;

	switch (this->fit) {
		case Fit::fill:
		case Fit::contain: {
			this->quad.setPos(pos);
			break;
		}
		case Fit::none:
		case Fit::cover: {
			this->quad.setPos(pos.withXOffset(-(quadSize.x - widgetSize.x) / 2.0f)
								  .withYOffset(-(quadSize.y - widgetSize.y) / 2.0f));
			break;
		}
	}
}

void Image::Impl::onDraw() {
	Renderer::getInstance().addClipRect(getContentRect());
	Renderer::getInstance().addQuad(quad);
	Renderer::getInstance().popClipRect();
}
