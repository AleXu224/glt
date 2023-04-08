#include "text.hpp"
#include "fontStore.hpp"
#include "renderer.hpp"

using namespace squi;

Text::Impl::Impl(const Text &args)
	: Widget(args.widget, Widget::Options{
        .shouldDrawChildren = false,
    }),
	  text(args.text),
	  fontSize(args.fontSize),
	  fontPath(args.fontPath),
	  color(args.color) {
	auto [quadsVec, width, height] = FontStore::generateQuads(text, fontPath, fontSize, 0, color);
    quads = std::move(quadsVec);
    auto &widgetData = data();
    widgetData.size = {width, height};
}

void Text::Impl::onUpdate() {
}

void Text::Impl::onDraw() {
    auto &widgetData = data();
    const auto pos = widgetData.pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset();
    if (pos.x != lastX || pos.y != lastY) {
		for (auto &quad: quads) {
			quad.setPos(pos);
		}
		lastX = pos.x;
        lastY = pos.y;
    }
    
    auto &renderer = Renderer::getInstance();
    
    for (auto &quad : quads) {
		renderer.addQuad(quad);
	}
}
