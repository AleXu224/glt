#include "box.hpp"
#include "renderer.hpp"

using namespace squi;

Box::Impl::Impl(const Box &args)
	: Widget(args.widget, Widget::Options{
		.isInteractive = true,
	}),
	  quad(Quad::Args{
		  .size = args.widget.size,
		  .color = args.color,
		  .borderColor = args.borderColor,
		  .borderRadius = args.borderRadius,
		  .borderSize = args.borderWidth,
		  .textureType = TextureType::NoTexture,
	  }) {
	addChild(args.child);
}

void Box::Impl::onDraw() {
	auto &data = this->data();
	quad.setPos(data.pos + data.margin.getPositionOffset());
	quad.setSize(data.size);

	auto &renderer = Renderer::getInstance();
	renderer.addQuad(quad);
}

void Box::Impl::setColor(const Color &color) {
	quad.setColor(color);
}

void Box::Impl::setBorderColor(const Color &color) {
	quad.setBorderColor(color);
}

void Box::Impl::setBorderWidth(float width) {
	quad.setBorderSize(width);
}

void Box::Impl::setBorderRadius(float radius) {
	quad.setBorderRadius(radius);
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
