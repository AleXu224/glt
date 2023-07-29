#include "text.hpp"
#include "fontStore.hpp"
#include "ranges"
#include "renderer.hpp"
#include <algorithm>
#include <string_view>


using namespace squi;

Text::Impl::Impl(const Text &args)
	: Widget(args.widget.withWidth(Size::Shrink).withHeight(Size::Shrink), Widget::Flags{
																			   .shouldDrawChildren = false,
																		   }),
	  text(args.text), fontSize(args.fontSize), lineWrap(args.lineWrap), fontPath(args.fontPath), color(args.color) {
	auto [quadsVec, width, height] = FontStore::generateQuads(text, fontPath, fontSize, 0, color);
	quads = std::move(quadsVec);
	textSize = {width, height};
	updateSize();
}

void Text::Impl::onLayout(vec2 &maxSize, vec2 &minSize) {
	if (lineWrap) {
		if (maxSize.x != lastAvailableSpace) {
			lastAvailableSpace = maxSize.x;
			const auto lineHeight = FontStore::getLineHeight(fontPath, fontSize);
			const auto rect = getRect();
			const auto padding = state.padding.getSizeOffset();
			// Only update the text layout if the text is wider than the available space
			if (rect.width() > maxSize.x || rect.height() != (static_cast<float>(lineHeight) + padding.y)) {
				auto [quadsVec, width, height] = FontStore::generateQuads(
					text,
					fontPath,
					fontSize,
					{lastX, lastY},
					color,
					maxSize.x - padding.x);
				textSize = {width, height};
				// minSize = {width + padding.x, height + padding.y};
				quads = std::move(quadsVec);
			}

			updateSize();
		}
	}

	// if (state.sizeMode.width.index() == 0) {
	// 	maxSize.x = std::get<0>(state.sizeMode.width);
	// }
	// if (state.sizeMode.height.index() == 0) {
	// 	maxSize.y = std::get<0>(state.sizeMode.height);
	// }
	minSize.y = textSize.y + state.padding.getSizeOffset().y;
	if (!lineWrap)
		maxSize = textSize + state.padding.getSizeOffset();
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
		state.sizeMode.width = Size::Expand;
		state.sizeMode.height = Size::Shrink;
	} else {
		const auto padding = state.padding.getSizeOffset();
		state.sizeMode.width = textSize.x + padding.x;
		state.sizeMode.height = textSize.y + padding.y;
	}
}

void Text::Impl::onDraw() {
	const auto pos = getPos() + state.margin.getPositionOffset() + state.padding.getPositionOffset();

	auto &renderer = Renderer::getInstance();
	const auto &clipRect = renderer.getCurrentClipRect().rect;
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
	this->text = text;
	auto [quadsVec, width, height] = FontStore::generateQuads(text, fontPath, fontSize, getPos().rounded(), color);
	quads = std::move(quadsVec);
	textSize = {width, height};
	updateSize();
}

std::string_view Text::Impl::getText() const {
	return text;
}

std::tuple<uint32_t, uint32_t> Text::Impl::getTextSize(const std::string_view &text) const {
	return FontStore::getTextSizeSafe(text, fontPath, fontSize);
}

float Text::Impl::getMinWidth(const vec2 &maxSize) {
	if (lineWrap) {
		const auto [width, height] = FontStore::getTextSizeSafe(text, fontPath, fontSize, maxSize.x);
		return static_cast<float>(width) + state.margin.getSizeOffset().x + state.padding.getSizeOffset().x;
	}
	return textSize.x + state.margin.getSizeOffset().x + state.padding.getSizeOffset().x;
}

float Text::Impl::getMinHeight(const vec2 &maxSize) {
	if (lineWrap) {
		const auto [width, height] = FontStore::getTextSizeSafe(text, fontPath, fontSize, maxSize.x);
		return static_cast<float>(height) + state.margin.getSizeOffset().y + state.padding.getSizeOffset().y;
	}
	return textSize.y + state.margin.getSizeOffset().y + state.padding.getSizeOffset().y;
}