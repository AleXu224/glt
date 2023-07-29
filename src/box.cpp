#include "box.hpp"
#include "renderer.hpp"

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
	  borderColor(args.borderColor), borderPosition(args.borderPosition), borderWidth(args.borderWidth), shouldClipContent(args.shouldClipContent) {
	addChild(args.child);
}

void Box::Impl::onDraw() {
	auto &renderer = Renderer::getInstance();
	renderer.addQuad(quad);
}

void Box::Impl::drawChildren() {
	auto &renderer = Renderer::getInstance();
	if (shouldClipContent)
		renderer.addClipRect(getRect().inset(borderWidth), quad.getData().borderRadius);

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
