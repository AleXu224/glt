#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <variant>
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "fontStore.hpp"
#include "ranges"
#include "renderer.hpp"
#include "text.hpp"
#include "widget.hpp"
#include <algorithm>
#include <string_view>


using namespace squi;

Text::Impl::Impl(const Text &args)
	: Widget(args.widget.withDefaultWidth(Size::Shrink).withDefaultHeight(Size::Shrink), Widget::Flags{
																							 .shouldDrawChildren = false,
																						 }),
	  text(args.text), fontSize(args.fontSize), lineWrap(args.lineWrap), font([&]() {
		  if (std::holds_alternative<std::string_view>(args.font)) {
			  return FontStore::getFont(std::get<std::string_view>(args.font));
		  } else {
			  return std::get<std::shared_ptr<FontStore::Font>>(args.font);
		  }
	  }()),
	  color(args.color) {
	auto [quadsVec, width, height] = font->generateQuads(text, fontSize, 0, color);
	quads = std::move(quadsVec);
	textSize = {width, height};
	updateSize();
}

// The text characters are considered to be the children of the text widget
vec2 Text::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) {
	if (shouldShrink.width && lineWrap) maxSize.x = 0;
	if (lineWrap && maxSize.x != lastAvailableSpace) {
		lastAvailableSpace = maxSize.x;
		const auto lineHeight = font->getLineHeight(fontSize);

		// Will only recalculate the text layout under the following circumstances:
		// 1. The available width is smaller than the cached text width
		// 2. The cached text is wrapping (the text is occupying more than one line)
		// - This is done because it would be really difficult to figure out if a change in available width would cause a layout change in this case
		if (maxSize.x < textSize.x || static_cast<uint32_t>(textSize.y) != lineHeight) {
			auto [quadsVec, width, height] = font->generateQuads(text, fontSize, {lastX, lastY}, color, maxSize.x);
			textSize = {width, height};
			quads = std::move(quadsVec);

			updateSize();
		}
	}

	return textSize;
}

void Text::Impl::onArrange(vec2 &pos) {
	const auto textPos = pos + state.margin.getPositionOffset() + state.padding.getPositionOffset();
	if (textPos.x != lastX || textPos.y != lastY) {
		const vec2 roundedPos = textPos.rounded();
		for (auto &quadVec: quads) {
			for (auto &quad: quadVec) {
				quad.setPos(roundedPos);
			}
		}
		lastX = textPos.x;
		lastY = textPos.y;
	}
}

void Text::Impl::updateSize() {
	if (lineWrap) {
		setWidth(Size::Expand);
		setHeight(Size::Shrink);
	} else {
		const auto newSize = textSize + state.padding.getSizeOffset();
		setWidth(newSize.x);
		setHeight(newSize.y);
	}
	reLayout();
}

void Text::Impl::onDraw() {
	const auto pos = getPos() + state.margin.getPositionOffset() + state.padding.getPositionOffset();

	auto &renderer = Renderer::getInstance();
	const auto clipRect = renderer.getCurrentClipRect().rect;
	const auto minOffsetX = clipRect.left - pos.x;
	// const auto minOffsetY = clipRect.top - pos.y;
	const auto maxOffsetX = clipRect.right - pos.x;
	// const auto maxOffsetY = clipRect.bottom - pos.y;

	for (auto &quadVec: quads) {
		auto it = std::lower_bound(
			quadVec.begin(),
			quadVec.end(),
			minOffsetX,
			[](const auto &quad, const auto &offset) {
				return (quad.getData().offset.x + quad.getData().size.x) < offset;
			});
		auto it2 = std::lower_bound(
			it,
			quadVec.end(),
			maxOffsetX,
			[](const auto &quad, const auto &offset) {
				return (quad.getData().offset.x) < offset;
			});
		for (auto &quad: std::ranges::subrange(it, it2)) {
			renderer.addQuad(quad);
		}
	}
}

void Text::Impl::setText(const std::string_view &text) {
	if (this->text == text) return;
	this->text = text;
	auto [quadsVec, width, height] = font->generateQuads(text, fontSize, getPos().rounded(), color);
	quads = std::move(quadsVec);
	if (textSize == vec2{width, height}) return;
	textSize = {width, height};
	updateSize();
}

std::string_view Text::Impl::getText() const {
	return text;
}

std::tuple<uint32_t, uint32_t> Text::Impl::getTextSize(const std::string_view &text) const {
	return font->getTextSizeSafe(text, fontSize);
}