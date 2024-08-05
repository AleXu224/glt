#include "networking.hpp"

#include "engine/compiledShaders/texturedRectfrag.hpp"
#include "engine/compiledShaders/texturedRectvert.hpp"
#include "filesystem"
#include "fstream"
#include "image.hpp"
#include "imageLoader.hpp"
#include "pipeline.hpp"
#include "samplerUniform.hpp"
#include "texture.hpp"
#include "texturedQuad.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <cstddef>
#include <cstring>
#include <expected>
#include <future>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>


#include "format"
#include "stdexcept"

using namespace squi;

// std::unique_ptr<Image::ImagePipeline> Image::Impl::pipeline = nullptr;
Image::ImagePipeline *Image::Impl::pipeline = nullptr;

Image::Data::Data(unsigned char *bytes, uint32_t length) : width(0), height(0) {
	auto res = loadImageInto(bytes, length);

	width = res.width;
	height = res.height;
	channels = res.channels;
	data = res.data;
}

Image::Data Image::Data::fromUrl(std::string_view url) {
	auto response = Networking::get(url);
	if (!response.success) {
		throw std::runtime_error(std::format("Failed to load image: {}", response.error));
	}
	return {reinterpret_cast<unsigned char *>(response.body.data()), (uint32_t) response.body.size()};
}

Image::Data Image::Data::fromFile(std::string_view path) {
	std::ifstream s{std::filesystem::path{path}, std::ios::binary};

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

	memcpy(ret.mappedMemory, this->data.data(), static_cast<size_t>(this->width) * this->height * this->channels);

	return ret;
}

Image::Impl::Impl(const Image &args)
	: Widget(
		  args.widget,
		  Widget::FlagsArgs{
			  .shouldLayoutChildren = false,
		  }
	  ),
	  fit(args.fit) {
	funcs().onInit.emplace_back([img = args.image](Widget &w) {
		std::thread imageCreatorThread{[img = img, w = w.weak_from_this()]() mutable {
			const Data &data = std::invoke([&]() -> const Data & {
				switch (img.index()) {
					case 0: {
						return std::get<0>(img);
					}
					case 1: {
						const auto &future = std::get<1>(img);
#ifdef _DEBUG
						if (!future.valid()) {
							std::println("Warning: Image future is not valid");
						}
#endif
						future.wait();
						return future.get();
					}
					default: {
						std::unreachable();
					}
				}
			});

			if (w.expired()) return;

			auto &widget = w.lock()->as<Image::Impl>();

			widget.sampler.emplace(Engine::SamplerUniform::Args{
				.instance = Window::of(&widget).engine.instance,
				.textureArgs{
					Engine::Texture::Args{
						.instance = Window::of(&widget).engine.instance,
						.width = static_cast<uint32_t>(data.width),
						.height = static_cast<uint32_t>(data.height),
						.channels = static_cast<uint32_t>(data.channels),
					},
				},
			});
			auto layout = widget.sampler->texture.image.getSubresourceLayout(vk::ImageSubresource{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.mipLevel = 0,
				.arrayLayer = 0,
			});
			for (uint32_t row = 0; row < data.height; row++) {
				memcpy(
					reinterpret_cast<uint8_t *>(widget.sampler->texture.mappedMemory) + row * layout.rowPitch,
					data.data.data() + static_cast<ptrdiff_t>(row * data.width * data.channels),
					static_cast<size_t>(data.width) * static_cast<size_t>(data.channels)
				);
			}
			widget.relayoutNextFrame = true;
		}};

		imageCreatorThread.detach();
	});
}

void Image::Impl::onUpdate() {
	if (relayoutNextFrame) {
		reLayout();
		relayoutNextFrame = false;
	}
}

void Image::Impl::onLayout(vec2 &maxSize, vec2 & /*minSize*/) {
	if (!sampler.has_value()) {
		return;
	}
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
	if (!sampler.has_value()) {
		return;
	}
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
	if (!sampler.has_value()) {
		return;
	}
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
	if (pipeline == nullptr) {
		auto &instance = Window::of(this).engine.instance;
		pipeline = &instance.createPipeline<ImagePipeline>(ImagePipeline::Args{
			.vertexShader = Engine::Shaders::texturedRectvert,
			.fragmentShader = Engine::Shaders::texturedRectfrag,
			.instance = Window::of(this).engine.instance,
		});
	}

	if (!sampler.has_value()) {
		return;
	}

	auto &window = Window::of(this);
	window.engine.instance.pushScissor(getRect());

	pipeline->bindWithSampler(sampler.value());
	auto [vi, ii] = pipeline->getIndexes();
	pipeline->addData(quad.getData(vi, ii));

	window.engine.instance.popScissor();
}
