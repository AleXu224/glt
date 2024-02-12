#include "networking.hpp"

#include "engine/compiledShaders/texturedRectfrag.hpp"
#include "engine/compiledShaders/texturedRectvert.hpp"
#include "fstream"
#include "image.hpp"
#include "pipeline.hpp"
#include "samplerUniform.hpp"
#include "texture.hpp"
#include "texturedQuad.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <cstring>
#include <expected>
#include <fstream>
#include <future>
#include <ios>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "../external/stb_image_ext.h"
#include "../external/stb_image_write.h"
#include "format"
#include "stdexcept"

using namespace squi;

// std::unique_ptr<Image::ImagePipeline> Image::Impl::pipeline = nullptr;
Image::ImagePipeline *Image::Impl::pipeline = nullptr;

Image::Data::Data(unsigned char *bytes, uint32_t length) {
	stbi_uc *loadedData = stbi_load_from_memory(bytes, (int) length, &width, &height, &channels, 0);
	if (loadedData == nullptr) {
		throw std::runtime_error("Failed to load image");
	}
	if (channels == 3) {
		channels = 4;
		this->data.resize(width * height * 4);
		for (int i = 0; i < width * height; i++) {
			this->data[i * 4 + 0] = loadedData[i * 3 + 0];
			this->data[i * 4 + 1] = loadedData[i * 3 + 1];
			this->data[i * 4 + 2] = loadedData[i * 3 + 2];
			this->data[i * 4 + 3] = 255;
		}
	} else if (channels == 1 || channels == 2 || channels == 4) {
		this->data.resize(width * height * channels);
		std::memcpy(this->data.data(), loadedData, width * height * channels);
	} else {
		throw std::runtime_error("Unsupported number of channels");
	}
	stbi_image_free(loadedData);
}

Image::Data Image::Data::fromUrl(std::string_view url) {
	auto response = Networking::get(url);
	if (!response.success) {
		throw std::runtime_error(std::format("Failed to load image: {}", response.error));
	}
	return {reinterpret_cast<unsigned char *>(response.body.data()), (uint32_t) response.body.size()};
}

Image::Data Image::Data::fromFile(std::string_view path) {
	std::ifstream s{path.data(), std::ios::binary | std::ios::in};

	if (!s) {
		std::cout << "Failed to open file " << path << std::endl;
		return {{0, 0, 0, 0}, 1, 1, 4};
	}

	std::stringstream bytes{};
	bytes << s.rdbuf();

	auto str = bytes.str();
	Image::Data data{reinterpret_cast<unsigned char *>(str.data()), static_cast<uint32_t>(str.size())};
	return data;
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

Engine::Texture Image::Data::createTexture(Engine::Instance &instance) const {
	Engine::Texture ret{Engine::Texture::Args{
		.instance = instance,
		.width = static_cast<uint32_t>(this->width),
		.height = static_cast<uint32_t>(this->height),
		.channels = static_cast<uint32_t>(this->channels),
	}};

	memcpy(ret.mappedMemory, this->data.data(), this->width * this->height * this->channels * sizeof(uint8_t));

	return ret;
}

Image::Impl::Impl(const Image &args)
	: Widget(args.widget, Widget::FlagsArgs{
							  .shouldLayoutChildren = false,
						  }),
	  fit(args.fit), type(args.type) {

	switch (args.image.index()) {
		case 0: {
			auto &data = std::get<0>(args.image);
			std::future<Data> texFuture = std::async(std::launch::async, [data = data]() -> Data {
				return data;
			});
			state.properties.insert({"imageFuture", texFuture.share()});
			break;
		}
		case 1: {
			auto &future = std::get<1>(args.image);
#ifdef _DEBUG
			if (!future.valid()) {
				printf("Warning: Image future is not valid\n");
			}
#endif
			std::future<Data> texFuture = std::async(std::launch::async, [future = future]() -> Data {
				future.wait();
				return future.get();
			});
			state.properties.insert({"imageFuture", texFuture.share()});
		}
	}
}

void Image::Impl::onUpdate() {
	if (sampler.has_value()) return;
	auto &future = std::any_cast<std::shared_future<Data> &>(state.properties.at("imageFuture"));
	// const auto status = future.wait_for(std::chrono::seconds(0));
	if (future._Is_ready()) {
		auto &data = future.get();
		sampler.emplace(Engine::SamplerUniform::Args{
			.instance = Window::of(this).engine.instance,
			.textureArgs{
				Engine::Texture::Args{
					.instance = Window::of(this).engine.instance,
					.width = static_cast<uint32_t>(data.width),
					.height = static_cast<uint32_t>(data.height),
					.channels = static_cast<uint32_t>(data.channels),
				},
			},
		});
		auto layout = sampler->texture.image.getSubresourceLayout(vk::ImageSubresource{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.mipLevel = 0,
			.arrayLayer = 0,
		});
		for (int row = 0; row < data.height; row++) {
			memcpy(
				(uint8_t *) sampler->texture.mappedMemory + row * layout.rowPitch,
				data.data.data() + row * data.width * data.channels,
				data.width * data.channels
			);
		}
		// stbi_write_png("hmmge.png", data.width, data.height, data.channels, sampler->texture.mappedMemory, data.width * data.channels);
		auto iter = state.properties.find("imageFuture");
		if (iter != state.properties.end()) {
			state.properties.erase(iter);
		}
		reLayout();
	}
}

void Image::Impl::onLayout(vec2 &maxSize, vec2 &minSize) {
	if (!sampler.has_value()) return;
	const auto &properties = sampler->texture;
	const float aspectRatio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
	switch (this->fit) {
		case Fit::none: {
			maxSize.x = std::min(maxSize.x, static_cast<float>(properties.width));
			maxSize.y = std::min(maxSize.y, static_cast<float>(properties.height));
			break;
		}
		case Fit::fill:
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
	if (!sampler.has_value()) return;
	const auto &properties = sampler->texture;

	switch (this->fit) {
		case Fit::none: {
			quad.size = vec2{
				static_cast<float>(properties.width),
				static_cast<float>(properties.height),
			};
			break;
		}
		case Fit::fill:
		case Fit::contain: {
			quad.size = size;
			break;
		}
		case Fit::cover: {
			const float aspectRatio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
			if (size.x / size.y > aspectRatio) {
				quad.size = vec2{
					size.x,
					size.x / aspectRatio,
				};
			} else {
				quad.size = vec2{
					size.y * aspectRatio,
					size.y,
				};
			}
			break;
		}
	}
}

void Image::Impl::postArrange(vec2 &pos) {
	if (!sampler.has_value()) return;
	const auto &widgetSize = getSize();
	const vec2 quadSize = vec2(quad.size);

	switch (this->fit) {
		case Fit::fill:
		case Fit::contain: {
			quad.position = pos;
			break;
		}
		case Fit::none:
		case Fit::cover: {
			quad.position = pos.withXOffset(-(quadSize.x - widgetSize.x) / 2.0f)
								.withYOffset(-(quadSize.y - widgetSize.y) / 2.0f);
			break;
		}
	}
}

void Image::Impl::onDraw() {
	if (!pipeline) {
		auto &instance = Window::of(this).engine.instance;
		// pipeline = std::make_unique<ImagePipeline>(ImagePipeline::Args{
		// 	.vertexShader = Engine::Shaders::texturedRectvert,
		// 	.fragmentShader = Engine::Shaders::texturedRectfrag,
		// 	.instance = Window::of(this).engine.instance,
		// });
		pipeline = &instance.createPipeline<ImagePipeline>(ImagePipeline::Args{
			.vertexShader = Engine::Shaders::texturedRectvert,
			.fragmentShader = Engine::Shaders::texturedRectfrag,
			.instance = Window::of(this).engine.instance,
		});
	}

	if (!sampler.has_value()) return;
	pipeline->bindWithSampler(sampler.value());
	auto [vi, ii] = pipeline->getIndexes();
	pipeline->addData(quad.getData(vi, ii));
}
