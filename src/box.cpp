#include "box.hpp"
#include "compiledShaders/rectfrag.hpp"
#include "compiledShaders/rectvert.hpp"
#include "pipeline.hpp"
#include "quad.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <memory>


using namespace squi;

Box::BoxPipeline *Box::Impl::pipeline = nullptr;

Box::Impl::Impl(const Box &args)
	: Widget(args.widget, Widget::FlagsArgs{
							  .isInteractive = true,
						  }),
	  quad(Engine::Quad::Args{
		  .size{0, 0},
		  .color = args.color,
		  .borderRadiuses = args.borderRadius,
		  .borderSizes = args.borderWidth,
		  .borderColor = args.borderPosition == BorderPosition::inset ? args.borderColor.mix(args.color) : args.borderColor,
	  }),
	  borderPosition(args.borderPosition), shouldClipContent(args.shouldClipContent) {
	addChild(args.child);
}

void Box::Impl::onDraw() {
	if (!pipeline) {
		Engine::Instance &instance = Window::of(this).engine.instance;

		pipeline = &instance.createPipeline<BoxPipeline>(BoxPipeline::Args{
			.vertexShader = Engine::Shaders::rectvert,
			.fragmentShader = Engine::Shaders::rectfrag,
			.instance = instance,
		});
	}
	pipeline->bind();
	auto index = pipeline->getIndexes();
	pipeline->addData(quad.getData(index.first, index.second));
}

void Box::Impl::drawChildren() {
	auto &window = Window::of(this);
	// FIXME: inset by border sizes
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
	quad.size = size;
}

void Box::Impl::postArrange(vec2 &pos) {
	quad.position = pos + state.margin->getPositionOffset();
}

void Box::Impl::setColor(const Color &color) {
	if (color == Color(quad.color)) return;
	quad.color = color;
	if (borderPosition == BorderPosition::inset) {
		quad.borderColor = Color(quad.borderColor).mix(color);
	}
	reDraw();
}

void Box::Impl::setBorderColor(const Color &color) {
	if (borderPosition == BorderPosition::inset) {
		const Color newColor = color.mix(Color(quad.color));
		if (newColor == Color(quad.borderColor)) return;
		quad.borderColor = newColor;
	} else {
		if (color == Color(quad.borderColor)) return;
		quad.borderColor = color;
	}
	reDraw();
}

void Box::Impl::setBorderWidth(glm::vec4 width) {
	if (width.x == quad.borderSizes.top &&
		width.y == quad.borderSizes.right &&
		width.z == quad.borderSizes.bottom &&
		width.w == quad.borderSizes.left) return;
	quad.borderSizes.top = width.x;
	quad.borderSizes.right = width.t;
	quad.borderSizes.bottom = width.z;
	quad.borderSizes.left = width.w;
	reDraw();
}

void Box::Impl::setBorderRadius(glm::vec4 radius) {
	if (radius.x == quad.borderRadiuses.topLeft &&
		radius.y == quad.borderRadiuses.topRight &&
		radius.z == quad.borderRadiuses.bottomRight &&
		radius.w == quad.borderRadiuses.bottomLeft) return;
	quad.borderRadiuses.topLeft = radius.x;
	quad.borderRadiuses.topRight = radius.t;
	quad.borderRadiuses.bottomRight = radius.z;
	quad.borderRadiuses.bottomLeft = radius.w;
	reDraw();
}

Color Box::Impl::getColor() const {
	return {quad.color};
}

Color Box::Impl::getBorderColor() const {
	return {quad.borderColor};
}

glm::vec4 Box::Impl::getBorderWidth() const {
	return {quad.borderSizes.all};
}

glm::vec4 Box::Impl::getBorderRadius() const {
	return {quad.borderRadiuses.all};
}

Engine::Quad &Box::Impl::getQuad() {
	return quad;
}
