#include "box.hpp"
#include "renderer.hpp"

using namespace squi;

Box::Box(const Box::Args &args)
	: Widget(args.widget, Widget::Options{.isContainer = false}),
	  quad(Quad::Args{
		  .size = args.widget.size,
		  .color = args.color,
		  .borderColor = args.borderColor,
		  .borderRadius = args.borderRadius,
		  .borderSize = args.borderWidth,
		  .textureId = 0,
		  .textureType = Quad::TextureType::NoTexture,
	  }) {
	addChild(args.child);
}

void Box::onDraw() {
	quad.setPos(getPos() + getMargin().getPositionOffset());
	quad.setSize(getSize());

	auto &renderer = Renderer::getInstance();
	renderer.addQuad(quad);
}

void Box::setColor(const Color &color) {
	quad.setColor(color);
}

void Box::setBorderColor(const Color &color) {
	quad.setBorderColor(color);
}

void Box::setBorderWidth(float width) {
	quad.setBorderSize(width);
}

void Box::setBorderRadius(float radius) {
	quad.setBorderRadius(radius);
}

Color Box::getColor() const {
	return Color::VEC4(quad.getData().color);
}

Color Box::getBorderColor() const {
	return Color::VEC4(quad.getData().borderColor);
}

float Box::getBorderWidth() const {
	return quad.getData().borderSize;
}

float Box::getBorderRadius() const {
	return quad.getData().borderRadius;
}

Quad &Box::getQuad() {
	return quad;
}
