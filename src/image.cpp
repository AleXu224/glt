#include "image.hpp"

#include "engine/compiledShaders/texturedRectfrag.hpp"
#include "engine/compiledShaders/texturedRectvert.hpp"
#include "imageData.hpp"
#include "pipeline.hpp"
#include "samplerUniform.hpp"
#include "store/texture.hpp"
#include "texture.hpp"
#include "texturedQuad.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "window.hpp"


#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <memory>
#include <vector>

using namespace squi;

Image::Impl::Impl(const Image &args)
	: Widget(
		  args.widget,
		  Widget::FlagsArgs{
			  .shouldLayoutChildren = false,
		  }
	  ),
	  fit(args.fit),
	  data(std::make_unique<ImageDataImpl>()) {
	funcs().onInit.emplace_back([img = args.image](Widget &w) {
		auto &image = w.as<Image::Impl>();
		auto &window = Window::of(&w);

		image.data->pipeline = window.pipelineStore.getPipeline(Store::PipelineProvider<ImagePipeline>{
			.key = "squiImagePipeline",
			.provider = [&]() {
				return ImagePipeline::Args{
					.vertexShader = Engine::Shaders::texturedRectvert,
					.fragmentShader = Engine::Shaders::texturedRectfrag,
					.instance = window.engine.instance,
				};
			},
		});

		image.data->sampler = window.samplerStore.getSampler(window.engine.instance, Store::Texture::getTexture(img));
	});
}

void Image::Impl::onUpdate() {
	if (relayoutNextFrame) {
		reLayout();
		relayoutNextFrame = false;
	}
}

void Image::Impl::onLayout(vec2 &maxSize, vec2 & /*minSize*/) {
	if (!data->sampler) {
		return;
	}
	const auto &properties = *data->sampler->texture;
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
	auto &sampler = data->sampler;
	auto &quad = data->quad;
	if (!sampler) {
		return;
	}
	const auto &properties = *sampler->texture;

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
	auto &sampler = data->sampler;
	auto &quad = data->quad;
	if (!sampler) {
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
	auto &pipeline = data->pipeline;
	auto &sampler = data->sampler;
	auto &quad = data->quad;
	if (!pipeline) return;

	if (!sampler) return;

	auto &window = Window::of(this);
	window.engine.instance.pushScissor(getRect());

	pipeline->bindWithSampler(*sampler);
	auto [vi, ii] = pipeline->getIndexes();
	pipeline->addData(quad.getData(vi, ii));

	window.engine.instance.popScissor();
}

squi::Image::Impl::~Impl() = default;
