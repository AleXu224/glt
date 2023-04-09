#include "text.hpp"
#include "fontStore.hpp"
#include "renderer.hpp"

using namespace squi;

Text::Impl::Impl(const Text &args)
	: Widget(args.widget, Widget::Options{
							  .shouldDrawChildren = false,
                .shouldHandleSizeBehavior = false,
						  }),
	  text(args.text), fontSize(args.fontSize), lineWrap(args.lineWrap), fontPath(args.fontPath), color(args.color) {
	auto [quadsVec, width, height] = FontStore::generateQuads(text, fontPath, fontSize, 0, color);
	quads = std::move(quadsVec);
	auto &widgetData = data();
	widgetData.size = {width, height};
}

void Text::Impl::onUpdate() {
  auto &widgetData = data();

  if (lineWrap) {
    const auto parentWidth = widgetData.parent->getContentRect().width();
    if (parentWidth != lastParentWidth) {
      lastParentWidth = parentWidth;
      const auto maxWidth = parentWidth - widgetData.margin.getSizeOffset().x - widgetData.padding.getSizeOffset().x;
      auto [quadsVec, width, height] = FontStore::generateQuads(text, fontPath, fontSize, {lastX, lastY}, color, maxWidth);
      widgetData.size = {width, height};
      quads = std::move(quadsVec);
    }
  }
}

void Text::Impl::onDraw() {
	auto &widgetData = data();
	const auto pos = widgetData.pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset();
	if (pos.x != lastX || pos.y != lastY) {
		for (auto &quadVec: quads) {
      for (auto &quad: quadVec) {
        quad.setPos(pos);
      }
		}
		lastX = pos.x;
		lastY = pos.y;
	}

	auto &renderer = Renderer::getInstance();
  const auto &clipRect = renderer.getCurrentClipRect();
  const auto minOffsetX = clipRect.left - widgetData.pos.x;
  const auto minOffsetY = clipRect.top - widgetData.pos.y;
  const auto maxOffsetX = clipRect.right - widgetData.pos.x;
  const auto maxOffsetY = clipRect.bottom - widgetData.pos.y;

	for (auto &quadVec: quads) {
    for (auto &quad: quadVec) {
      const auto &quadData = quad.getData();
      if (quadData.offset.x > maxOffsetX || quadData.offset.y > maxOffsetY)
        break;
      if (quadData.offset.x + quadData.size.x < minOffsetX || quadData.offset.y + quadData.size.y < minOffsetY)
        continue;
      renderer.addQuad(quad);
    }
	}
}
