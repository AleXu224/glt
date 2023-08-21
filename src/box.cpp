#include "box.hpp"
#include "renderer.hpp"
#include "widget.hpp"

using namespace squi;

Box::Impl::Impl(const Box &args)
	: Widget(args.widget, Widget::Flags{
							  .isInteractive = true,
						  }),
	  quad(Quad::Args{
		  .size = vec2(0, 0),
		  .color = args.color,
		  .borderColor = args.borderPosition == BorderPosition::inset ? args.borderColor.mix(args.color) : args.borderColor,
		  .borderRadius = args.borderRadius,
		  .borderSize = args.borderWidth,
		  .textureType = TextureType::NoTexture,
	  }),
	  borderPosition(args.borderPosition), shouldClipContent(args.shouldClipContent) {
	addChild(args.child);
}

void Box::Impl::onDraw() {
	auto &renderer = Renderer::getInstance();
	renderer.addQuad(quad);
}

void Box::Impl::drawChildren() {
	auto &renderer = Renderer::getInstance();
	const auto &quadData = quad.getData();
	if (shouldClipContent)
		renderer.addClipRect(getRect().inset(quadData.borderSize), quadData.borderRadius);

	auto &children = getChildren();
	for (auto &child: children) {
		child->draw();
	}

	if (shouldClipContent)
		renderer.popClipRect();
}

void Box::Impl::postLayout(vec2 &size) {
	quad.setSize(size);
}

void Box::Impl::postArrange(vec2 &pos) {
	quad.setPos(pos + state.margin.getPositionOffset());
}

void Box::Impl::setColor(const Color &color) {
	if (color == Color::VEC4(quad.getData().color)) return;
	quad.setColor(color);
	if (borderPosition == BorderPosition::inset) {
		quad.setBorderColor(Color::VEC4(quad.getData().borderColor).mix(color));
	}
	reDraw();
}

void Box::Impl::setBorderColor(const Color &color) {
	if (borderPosition == BorderPosition::inset) {
		const Color newColor = color.mix(Color::VEC4(quad.getData().color));
		if (newColor == Color::VEC4(quad.getData().borderColor)) return;
		quad.setBorderColor(newColor);
	} else {
		if (color == Color::VEC4(quad.getData().borderColor)) return;
		quad.setBorderColor(color);
	}
	reDraw();
}

void Box::Impl::setBorderWidth(float width) {
	if (width == quad.getData().borderSize) return;
	quad.setBorderSize(width);
	reDraw();
}

void Box::Impl::setBorderRadius(float radius) {
	if (radius == quad.getData().borderRadius) return;
	quad.setBorderRadius(radius);
	reDraw();
}

Color Box::Impl::getColor() const {
	return Color::VEC4(quad.getData().color);
}

Color Box::Impl::getBorderColor() const {
	return Color::VEC4(quad.getData().borderColor);
}

float Box::Impl::getBorderWidth() const {
	return quad.getData().borderSize;
}

float Box::Impl::getBorderRadius() const {
	return quad.getData().borderRadius;
}

Quad &Box::Impl::getQuad() {
	return quad;
}
