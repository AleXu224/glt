#include "box.hpp"

#include "boxData.hpp"
#include "compiledShaders/rectfrag.hpp"
#include "compiledShaders/rectvert.hpp"
#include "core/app.hpp"
#include "engine/pipeline.hpp"
#include "engine/quad.hpp"


namespace squi {
	Box::BoxRenderObject::BoxRenderObject() : data(std::make_unique<BoxData>(Engine::Quad::Args{})) {}

	void Box::BoxRenderObject::init() {
		auto *app = this->getApp();

		this->getWidgetAs<Box>()->updateRenderObject(this);

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

		data->quad.size = size.rounded();
		data->quad.position = pos.rounded();

		data->pipeline->bind();
		auto index = data->pipeline->getIndexes();
		data->pipeline->addData(data->quad.getData(index.first, index.second));
	}

	std::shared_ptr<RenderObject> Box::createRenderObject() {
		return std::make_shared<BoxRenderObject>();
	}

	void Box::updateRenderObject(RenderObject *renderObject) const {
		if (auto *boxRenderObject = dynamic_cast<BoxRenderObject *>(renderObject)) {
			auto *app = renderObject->getApp();

			auto &quad = boxRenderObject->data->quad;
			if (*quad.color != static_cast<glm::vec4>(this->color)) {
				quad.color = this->color;
				app->needsRedraw = true;
			}
			auto newBorderColor = this->borderPosition == BorderPosition::inset ? this->borderColor.mix(this->color) : this->borderColor;
			if (*quad.borderColor != static_cast<glm::vec4>(newBorderColor)) {
				quad.borderColor = newBorderColor;
				app->needsRedraw = true;
			}

			if (this->borderRadius.topLeft != quad.borderRadiuses.topLeft
				|| this->borderRadius.topRight != quad.borderRadiuses.topRight
				|| this->borderRadius.bottomRight != quad.borderRadiuses.bottomRight
				|| this->borderRadius.bottomLeft != quad.borderRadiuses.bottomLeft) {
				quad.borderRadiuses.topLeft = this->borderRadius.topLeft;
				quad.borderRadiuses.topRight = this->borderRadius.topRight;
				quad.borderRadiuses.bottomRight = this->borderRadius.bottomRight;
				quad.borderRadiuses.bottomLeft = this->borderRadius.bottomLeft;

				app->needsRedraw = true;
			}

			if (this->borderWidth.top != quad.borderSizes.top
				|| this->borderWidth.right != quad.borderSizes.right
				|| this->borderWidth.bottom != quad.borderSizes.bottom
				|| this->borderWidth.left != quad.borderSizes.left) {
				quad.borderSizes.top = this->borderWidth.top;
				quad.borderSizes.right = this->borderWidth.right;
				quad.borderSizes.bottom = this->borderWidth.bottom;
				quad.borderSizes.left = this->borderWidth.left;

				app->needsRedraw = true;
			}
		}
	}
}// namespace squi