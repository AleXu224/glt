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
			quad.color = this->color;
			quad.borderColor = this->borderPosition == BorderPosition::inset ? this->borderColor.mix(this->color) : this->borderColor;

			quad.borderRadiuses.topLeft = this->borderRadius.topLeft;
			quad.borderRadiuses.topRight = this->borderRadius.topRight;
			quad.borderRadiuses.bottomRight = this->borderRadius.bottomRight;
			quad.borderRadiuses.bottomLeft = this->borderRadius.bottomLeft;

			quad.borderSizes.top = this->borderWidth.top;
			quad.borderSizes.right = this->borderWidth.right;
			quad.borderSizes.bottom = this->borderWidth.bottom;
			quad.borderSizes.left = this->borderWidth.left;
		}
	}
}// namespace squi