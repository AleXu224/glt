#include "image.hpp"

#include "core/app.hpp"
#include "engine/compiledShaders/texturedRectfrag.hpp"
#include "engine/compiledShaders/texturedRectvert.hpp"
#include "imageData.hpp"
#include "store/texture.hpp"

namespace squi {
	Image::ImageRenderObject::ImageRenderObject() : data(std::make_unique<ImageDataImpl>(glt::Engine::TexturedQuad::Args{})) {}

	void Image::ImageRenderObject::init() {
		auto *app = this->getApp();

		this->getWidgetAs<Image>()->updateRenderObject(this);

		this->data->pipeline = app->pipelineStore.getPipeline(Store::PipelineProvider<ImagePipeline>{
			.key = "squiImagePipeline",
			.provider = [&]() {
				return ImagePipeline::Args{
					.vertexShader = glt::Engine::Shaders::texturedRectvert,
					.fragmentShader = glt::Engine::Shaders::texturedRectfrag,
					.instance = app->engine.instance,
				};
			},
		});
	}

	vec2 Image::ImageRenderObject::calculateContentSize(BoxConstraints constraints, bool) {
		if (!data->sampler) return {};

		const auto &properties = *data->sampler->texture;
		const float aspectRatio = static_cast<float>(properties.width) / static_cast<float>(properties.height);

		switch (this->fit) {
			case Fit::none: {
				return vec2{
					static_cast<float>(properties.width),
					static_cast<float>(properties.height),
				};
			}
			case Fit::fill:
			case Fit::cover: {
				return {};
			}
			case Fit::contain: {
				if (constraints.maxWidth / constraints.maxHeight > aspectRatio) {
					return vec2{
						constraints.maxHeight * aspectRatio,
						constraints.maxHeight,
					};
				}
				return vec2{
					constraints.maxWidth,
					constraints.maxWidth / aspectRatio,
				};
			}
		}
	}


	void Image::ImageRenderObject::drawSelf() {
		if (!data->pipeline) return;
		if (!data->sampler) return;

		const auto &properties = *data->sampler->texture;
		const float aspectRatio = static_cast<float>(properties.width) / static_cast<float>(properties.height);

		const auto widgetSize = this->size;
		vec2 size;
		switch (this->fit) {
			case Fit::none: {
				size = vec2{
					static_cast<float>(properties.width),
					static_cast<float>(properties.height),
				};
				break;
			}
			case Fit::fill: {
				size = widgetSize;
				break;
			}
			case Fit::cover: {
				if (widgetSize.x / widgetSize.y > aspectRatio) {
					size = vec2{
						widgetSize.x,
						widgetSize.x / aspectRatio,
					};
				} else {
					size = vec2{
						widgetSize.y * aspectRatio,
						widgetSize.y,
					};
				}
				break;
			}
			case Fit::contain: {
				if (widgetSize.x / widgetSize.y > aspectRatio) {
					size = vec2{
						widgetSize.y * aspectRatio,
						widgetSize.y,
					};
				} else {
					size = vec2{
						widgetSize.x,
						widgetSize.x / aspectRatio,
					};
				}
				break;
			}
		}

		data->quad.size = size;
		data->quad.position = getRect().posFromAlignment(Alignment::Center, Rect::fromPosSize(pos, size));

		data->pipeline->bindWithSampler(*data->sampler);
		auto index = data->pipeline->getIndexes();
		data->pipeline->addData(data->quad.getData(index.first, index.second));
	}

	std::shared_ptr<RenderObject> Image::createRenderObject() {
		return std::make_shared<ImageRenderObject>();
	}

	void Image::updateRenderObject(RenderObject *renderObject) const {
		if (auto *imageRenderObject = dynamic_cast<ImageRenderObject *>(renderObject)) {
			auto *app = renderObject->getApp();

			if (imageRenderObject->fit != this->fit) {
				imageRenderObject->fit = this->fit;
				app->needsRedraw = true;
			}

			if (imageRenderObject->imageProvider != this->image) {
				imageRenderObject->imageProvider = this->image;
				auto imageLoadingThread = std::thread([renderObject = renderObject->shared_from_this(), image = image]() {
					if (auto *imageRenderObject = renderObject->as<ImageRenderObject>()) {
						auto *app = renderObject->getApp();
						imageRenderObject->data->sampler = app->samplerStore.getSampler(app->engine.instance, Store::Texture::getTexture(image));
						std::scoped_lock _{app->taskMtx};
						app->preUpdateTasks.emplace_back([app]() {
							app->needsRelayout = true;
							app->inputQueue.push(StateChange{});
						});
					}
				});

				imageLoadingThread.detach();
			}
		}
	}
}// namespace squi