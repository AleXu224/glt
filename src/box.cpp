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
	  }) {}

void Box::onDraw() {
	quad.setPos(getPos() + getMargin().getPositionOffset());
	quad.setSize(getSize());

	auto &renderer = Renderer::getInstance();
	renderer.addQuad(quad);
}
