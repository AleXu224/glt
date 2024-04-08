#include "text.hpp"
#include "engine/compiledShaders/textRectfrag.hpp"
#include "engine/compiledShaders/textRectvert.hpp"
#include "fontStore.hpp"
#include "ranges"
#include "textQuad.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <algorithm>
#include <memory>
#include <string_view>
#include <tuple>
#include <variant>


using namespace squi;

// std::unique_ptr<Text::TextPipeline> Text::Impl::pipeline = nullptr;
Text::TextPipeline *Text::Impl::pipeline = nullptr;

Text::Impl::Impl(const Text &args)
	: Widget(
		  args.widget
			  .withDefaultWidth(Size::Shrink)
			  .withDefaultHeight(Size::Shrink),
		  Widget::FlagsArgs{
			  .shouldDrawChildren = false,
		  }
	  ),
	  text(args.text), fontSrc([&]() -> std::string_view {
		  if (std::holds_alternative<std::string_view>(args.font)) {
			  return std::get<std::string_view>(args.font);
		  }
		  return "";
	  }()),
	  fontSize(args.fontSize), lineWrap(args.lineWrap), color(args.color) {
	if (std::holds_alternative<std::shared_ptr<FontStore::Font>>(args.font)) {
		font = std::get<std::shared_ptr<FontStore::Font>>(args.font);
		forceRegen = true;
	}
}

// The text characters are considered to be the children of the text widget
vec2 Text::Impl::layoutChildren(vec2 maxSize, vec2 /*minSize*/, ShouldShrink shouldShrink) {
	if (!font.has_value()) {
		font = FontStore::getFont(fontSrc, Window::of(this).engine.instance);
		const auto textPos = (getPos() + state.margin->getPositionOffset() + state.padding->getPositionOffset()).rounded();
		std::tie(quads, textSize.x, textSize.y) = font.value()->generateQuads(text, fontSize, textPos, color);
	}
	if (shouldShrink.width && lineWrap) maxSize.x = 0;
	if ((lineWrap && maxSize.x != lastAvailableSpace) || forceRegen) {
		lastAvailableSpace = maxSize.x;
		const auto lineHeight = font.value()->getLineHeight(fontSize);

		// Will only recalculate the text layout under the following circumstances:
		// 1. The available width is smaller than the cached text width
		// 2. The cached text is wrapping (the text is occupying more than one line)
		// - This is done because it would be really difficult to figure o1ut if a change in available width would cause a layout change in this case
		if (maxSize.x < textSize.x || static_cast<uint32_t>(textSize.y) != lineHeight || forceRegen) {
			std::tie(quads, textSize.x, textSize.y) = font.value()->generateQuads(
				text,
				fontSize,
				vec2(lastX, lastY).rounded(),
				color,
				lineWrap ? std::optional<float>(maxSize.x) : std::nullopt
			);
			updateSize();
		}
	}

	return textSize + state.padding->getSizeOffset();
}

void Text::Impl::onArrange(vec2 &pos) {
	const auto textPos = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
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
		state.width = Size::Expand;
	} else {
		state.width = Size::Shrink;
	}
	reLayout();
}

void Text::Impl::onDraw() {
	if (!pipeline) {
		auto &instance = Window::of(this).engine.instance;
		pipeline = &instance.createPipeline<TextPipeline>(TextPipeline::Args{
			.vertexShader = Engine::Shaders::textRectvert,
			.fragmentShader = Engine::Shaders::textRectfrag,
			.instance = Window::of(this).engine.instance,
		});
	}

	const auto pos = (getPos() + state.margin->getPositionOffset() + state.padding->getPositionOffset()).rounded();

	pipeline->bindWithSampler(font.value()->getSampler());
	const auto clipRect = Window::of(this).engine.instance.scissorStack.back();
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
				return (quad.getOffset().x + quad.getSize().x) < offset;
			}
		);
		auto it2 = std::lower_bound(
			it,
			quadVec.end(),
			maxOffsetX,
			[](const auto &quad, const auto &offset) {
				return (quad.getOffset().x) < offset;
			}
		);
		for (auto &quad: std::ranges::subrange(it, it2)) {
			auto [vi, ii] = pipeline->getIndexes();
			pipeline->addData(quad.getData(vi, ii));
		}
	}
}

void Text::Impl::setText(const std::string_view &text) {
	if (this->text == text) return;
	this->text = text;
	forceRegen = true;
	reLayout();
}

std::string_view Text::Impl::getText() const {
	return text;
}

std::tuple<uint32_t, uint32_t> Text::Impl::getTextSize(const std::string_view &text) const {
	return font.value()->getTextSizeSafe(text, fontSize);
}

void squi::Text::Impl::setColor(const Color &newColor) {
	if (color == newColor) return;
	color = newColor;
	for (auto &quadVec: quads) {
		for (auto &quad: quadVec) {
			quad.setColor(newColor);
		}
	}
	reDraw();
}

const std::vector<std::vector<Engine::TextQuad>> &squi::Text::Impl::getQuads() const {
	return quads;
}

uint32_t squi::Text::Impl::getLineHeight() const {
	if (!font.has_value()) return 0;
	return font.value()->getLineHeight(fontSize);
}
