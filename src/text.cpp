#include "text.hpp"
#include "engine/compiledShaders/textRectfrag.hpp"
#include "engine/compiledShaders/textRectvert.hpp"
#include "engine/pipeline.hpp"
#include "engine/textQuad.hpp"
#include "fontStore.hpp"
#include "ranges"
#include "textData.hpp"
#include "textQuad.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <algorithm>
#include <memory>
#include <string_view>
#include <tuple>
#include <variant>


using namespace squi;

Text::Impl::Impl(const Text &args)
	: Widget(
		  args.widget
			  .withDefaultWidth(Size::Wrap)
			  .withDefaultHeight(Size::Shrink),
		  Widget::FlagsArgs{
			  .shouldDrawChildren = false,
		  }
	  ),
	  text(args.text),
	  fontSize(args.fontSize),
	  lineWrap(args.lineWrap),
	  color(args.color),
	  data(std::make_unique<TextData>()) {
	std::visit(
		[&](auto &&val) {
			using T = std::remove_cvref_t<decltype(val)>;
			if constexpr (std::is_same_v<T, FontProvider>) {
				font = FontStore::getFont(val);
			} else if constexpr (std::is_same_v<T, std::shared_ptr<FontStore::Font>>) {
				font = val;
			}
		},
		args.font
	);
	funcs().onInit.emplace_back([font = args.font](Widget &w) {
		auto &text = w.as<Text::Impl>();
		auto &window = Window::of(&w);

		text.data->pipeline = window.pipelineStore.getPipeline(Store::PipelineProvider<TextPipeline>{
			.key = "squiTextPipeline",
			.provider = [&]() {
				return TextPipeline::Args{
					.vertexShader = Engine::Shaders::textRectvert,
					.fragmentShader = Engine::Shaders::textRectfrag,
					.instance = window.engine.instance,
				};
			},
		});

		text.data->sampler = window.samplerStore.getSampler(window.engine.instance, text.font->getTexture());
		text.forceRegen = true;
	});
}

// The text characters are considered to be the children of the text widget
vec2 Text::Impl::layoutChildren(vec2 maxSize, vec2 /*minSize*/, ShouldShrink shouldShrink, bool /*final*/) {
	if (shouldShrink.width && lineWrap) maxSize.x = 0;
	if (lineWrap || forceRegen) {
		const auto &[width, height] = font->getTextSizeSafe(
			text,
			fontSize,
			lineWrap ? std::optional<float>(maxSize.x) : std::nullopt
		);
		return vec2{static_cast<float>(width), static_cast<float>(height)} + state.padding->getSizeOffset();
	}

	return textSize + state.padding->getSizeOffset();
}

squi::Text::Impl::~Impl() = default;

void squi::Text::Impl::postLayout(vec2 &size) {
	if ((lineWrap && size.x != lastAvailableSpace) || forceRegen) {
		lastAvailableSpace = size.x;
		const auto lineHeight = font->getLineHeight(fontSize);

		// Will only recalculate the text layout under the following circumstances:
		// 1. The available width is smaller than the cached text width
		// 2. The cached text is wrapping (the text is occupying more than one line)
		// - This is done because it would be really difficult to figure out if a change in available width would cause a layout change in this case
		if (size.x < textSize.x || static_cast<uint32_t>(textSize.y) != lineHeight || forceRegen) {
			std::tie(data->quads, textSize.x, textSize.y) = font->generateQuads(
				text,
				fontSize,
				vec2(lastX, lastY).rounded(),
				color,
				lineWrap ? std::optional<float>(size.x) : std::nullopt
			);
			updateSize();
		}
	}

	forceRegen = false;
}

void Text::Impl::onArrange(vec2 &pos) {
	const auto textPos = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
	if (textPos.x != lastX || textPos.y != lastY) {
		const vec2 roundedPos = textPos.rounded();
		for (auto &quadVec: data->quads) {
			for (auto &quad: quadVec) {
				quad.setPos(roundedPos);
			}
		}
		lastX = textPos.x;
		lastY = textPos.y;
	}
}

void Text::Impl::updateSize() {
	reLayout();
}

void Text::Impl::onDraw() {
	if (!data->pipeline) return;
	if (!data->sampler) return;

	const auto pos = (getPos() + state.margin->getPositionOffset() + state.padding->getPositionOffset()).rounded();

	data->pipeline->bindWithSampler(*data->sampler);
	const auto clipRect = Window::of(this).engine.instance.scissorStack.back();
	const auto minOffsetX = clipRect.left - pos.x;
	// const auto minOffsetY = clipRect.top - pos.y;
	const auto maxOffsetX = clipRect.right - pos.x;
	// const auto maxOffsetY = clipRect.bottom - pos.y;

	for (auto &quadVec: data->quads) {
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
			auto [vi, ii] = data->pipeline->getIndexes();
			data->pipeline->addData(quad.getData(vi, ii));
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
	return font->getTextSizeSafe(text, fontSize, lineWrap ? std::optional<float>(getContentSize().x) : std::nullopt);
}

void squi::Text::Impl::setColor(const Color &newColor) {
	if (color == newColor) return;
	color = newColor;
	for (auto &quadVec: data->quads) {
		for (auto &quad: quadVec) {
			quad.setColor(newColor);
		}
	}
	reDraw();
}

const TextData &squi::Text::Impl::getData() const {
	return *data;
}

uint32_t squi::Text::Impl::getLineHeight() const {
	return font->getLineHeight(fontSize);
}
