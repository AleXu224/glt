#include "box.hpp"

#include "boxData.hpp"
#include "compiledShaders/rectfrag.hpp"
#include "compiledShaders/rectvert.hpp"
#include "engine/pipeline.hpp"
#include "engine/quad.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <memory>


using namespace squi;

Box::Impl::Impl(const Box &args)
	: Widget(args.widget, Widget::FlagsArgs{
							  .isInteractive = true,
						  }),
	  data(std::make_unique<BoxData>(Engine::Quad::Args{
		  .size{0, 0},
		  .color = args.color,
		  .borderRadiuses = args.borderRadius,
		  .borderSizes = args.borderWidth,
		  .borderColor = args.borderPosition == BorderPosition::inset ? args.borderColor.mix(args.color) : args.borderColor,
	  })),
	  borderPosition(args.borderPosition), shouldClipContent(args.shouldClipContent) {
	addChild(args.child);

	funcs().onInit.emplace_back([](Widget &w) {
		auto &box = w.as<Box::Impl>();
		auto &window = Window::of(&w);

		box.data->pipeline = window.pipelineStore.getPipeline(Store::PipelineProvider<BoxPipeline>{
			.key = "squiBoxPipeline",
			.provider = [&]() {
				return BoxPipeline::Args{
					.vertexShader = Engine::Shaders::rectvert,
					.fragmentShader = Engine::Shaders::rectfrag,
					.instance = window.engine.instance,
				};
			},
		});
	});
}

void Box::Impl::onDraw() {
	if (!data->pipeline) return;
	data->pipeline->bind();
	auto index = data->pipeline->getIndexes();
	data->pipeline->addData(data->quad.getData(index.first, index.second));
}

void Box::Impl::drawChildren() {
	auto &window = Window::of(this);
	if (shouldClipContent) {
		window.engine.instance.pushScissor(getRect());
	}

	auto &children = getChildren();
	for (auto &child: children) {
		child->draw();
	}

	if (shouldClipContent) {
		window.engine.instance.popScissor();
	}
}

void Box::Impl::postLayout(vec2 &size) {
	data->quad.size = size;
}

void Box::Impl::postArrange(vec2 &pos) {
	data->quad.position = pos + state.margin->getPositionOffset();
}

void Box::Impl::setColor(const Color &color) {
	if (color == Color(data->quad.color)) return;
	data->quad.color = color;
	if (borderPosition == BorderPosition::inset) {
		data->quad.borderColor = Color(data->quad.borderColor).mix(color);
	}
	reDraw();
}

void Box::Impl::setBorderColor(const Color &color) {
	if (borderPosition == BorderPosition::inset) {
		const Color newColor = color.mix(Color(data->quad.color));
		if (newColor == Color(data->quad.borderColor)) return;
		data->quad.borderColor = newColor;
	} else {
		if (color == Color(data->quad.borderColor)) return;
		data->quad.borderColor = color;
	}
	reDraw();
}

void Box::Impl::setBorderWidth(BorderWidth width) {
	auto &qWidth = data->quad.borderSizes;
	if (width.top == qWidth.top
		&& width.right == qWidth.right
		&& width.bottom == qWidth.bottom
		&& width.left == qWidth.left) return;
	qWidth.top = width.top;
	qWidth.right = width.right;
	qWidth.bottom = width.bottom;
	qWidth.left = width.left;
	reDraw();
}

void Box::Impl::setBorderRadius(BorderRadius radius) {
	auto &qRadius = data->quad.borderRadiuses;
	if (radius.topLeft == qRadius.topLeft
		&& radius.topRight == qRadius.topRight
		&& radius.bottomRight == qRadius.bottomRight
		&& radius.bottomLeft == qRadius.bottomLeft) return;
	qRadius.topLeft = radius.topLeft;
	qRadius.topRight = radius.topRight;
	qRadius.bottomRight = radius.bottomRight;
	qRadius.bottomLeft = radius.bottomLeft;
	reDraw();
}

Color Box::Impl::getColor() const {
	return {data->quad.color};
}

Color Box::Impl::getBorderColor() const {
	return {data->quad.borderColor};
}

glm::vec4 Box::Impl::getBorderWidth() const {
	return {data->quad.borderSizes.all};
}

glm::vec4 Box::Impl::getBorderRadius() const {
	return {data->quad.borderRadiuses.all};
}

BoxData &Box::Impl::getData() {
	return *data;
}

squi::Box::Impl::~Impl() = default;
