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
		  .borderColor = args.borderPosition == BorderPosition::inset ? args.borderColor.mix(args.color) : args.borderColor,
		  .borderRadius = args.borderRadius,
		  .borderSize = args.borderWidth,
		  .textureType = TextureType::NoTexture,
	  }),
	  borderColor(args.borderColor), borderPosition(args.borderPosition), shouldClipContent(args.shouldClipContent) {
	addChild(args.child);
}

void Box::Impl::onDraw() {
	auto &data = this->data();
	quad.setPos(data.pos + data.margin.getPositionOffset());
	quad.setSize(data.size);

	auto &renderer = Renderer::getInstance();
	if (shouldClipContent)
		renderer.addClipRect(getRect(), quad.getData().borderRadius);
	renderer.addQuad(quad);
	if (shouldClipContent)
		renderer.popClipRect();
}

void Box::Impl::setColor(const Color &color) {
	quad.setColor(color);
	if (borderPosition == BorderPosition::inset) {
		quad.setBorderColor(borderColor.mix(color));
	}
}

void Box::Impl::setBorderColor(const Color &color) {
	if (borderPosition == BorderPosition::inset) {
		const auto quadColor = quad.getData().color;
		quad.setBorderColor(color.mix(Color::RGBA(quadColor.x, quadColor.y, quadColor.z, quadColor.w)));
	} else {
		quad.setBorderColor(color);
	}
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
