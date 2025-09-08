#include "box.hpp"

#include "boxData.hpp"
#include "compiledShaders/rectfrag.hpp"
#include "compiledShaders/rectvert.hpp"
#include "core/app.hpp"
#include "engine/pipeline.hpp"
#include "engine/quad.hpp"


namespace squi {
	Box::BoxRenderObject::BoxRenderObject(Args args)
		: core::SingleChildRenderObject(args),
		  data(std::make_unique<BoxData>(Engine::Quad::Args{})) {}

	void Box::BoxRenderObject::init() {
		auto app = this->getApp();

		this->data->pipeline = app->pipelineStore.getPipeline(Store::PipelineProvider<BoxPipeline>{
			.key = "squiBoxPipeline",
			.provider = [&]() {
				return BoxPipeline::Args{
					.vertexShader = Engine::Shaders::rectvert,
					.fragmentShader = Engine::Shaders::rectfrag,
					.instance = app->engine.instance,
				};
			},
		});
	}

	void Box::BoxRenderObject::drawSelf() {
		if (!data->pipeline) return;

		data->quad.size = size;
		data->quad.position = pos;

		data->pipeline->bind();
		auto index = data->pipeline->getIndexes();
		data->pipeline->addData(data->quad.getData(index.first, index.second));
	}

	std::shared_ptr<RenderObject> Box::createRenderObject() const {
		auto ret = std::make_shared<BoxRenderObject>(widget);
		this->updateRenderObject(ret.get());
		return ret;
	}

	void Box::updateRenderObject(RenderObject *renderObject) const {
		if (auto boxRenderObject = dynamic_cast<BoxRenderObject *>(renderObject)) {
			auto &quad = boxRenderObject->data->quad;
			// .size{0, 0},
			// 	.color = args.color,
			// 	.borderRadiuses = args.borderRadius,
			// 	.borderSizes = args.borderWidth,
			// 	.borderColor = args.borderPosition == BorderPosition::inset ? args.borderColor.mix(args.color) : args.borderColor,
			quad.color = this->color;

			// auto &qRadius = data->quad.borderRadiuses;
			// if (radius.topLeft == qRadius.topLeft
			// 	&& radius.topRight == qRadius.topRight
			// 	&& radius.bottomRight == qRadius.bottomRight
			// 	&& radius.bottomLeft == qRadius.bottomLeft) return;
			// qRadius.topLeft = radius.topLeft;
			// qRadius.topRight = radius.topRight;
			// qRadius.bottomRight = radius.bottomRight;
			// qRadius.bottomLeft = radius.bottomLeft;
			quad.borderRadiuses.topLeft = this->borderRadius.topLeft;
			quad.borderRadiuses.topRight = this->borderRadius.topRight;
			quad.borderRadiuses.bottomRight = this->borderRadius.bottomRight;
			quad.borderRadiuses.bottomLeft = this->borderRadius.bottomLeft;

			// auto &qWidth = data->quad.borderSizes;
			// if (width.top == qWidth.top
			// 	&& width.right == qWidth.right
			// 	&& width.bottom == qWidth.bottom
			// 	&& width.left == qWidth.left) return;
			// qWidth.top = width.top;
			// qWidth.right = width.right;
			// qWidth.bottom = width.bottom;
			// qWidth.left = width.left;
			quad.borderSizes.top = this->borderWidth.top;
			quad.borderSizes.right = this->borderWidth.right;
			quad.borderSizes.bottom = this->borderWidth.bottom;
			quad.borderSizes.left = this->borderWidth.left;

			quad.borderColor = this->borderPosition == BorderPosition::inset ? this->borderColor.mix(this->color) : this->borderColor;
		}
	}
}// namespace squi