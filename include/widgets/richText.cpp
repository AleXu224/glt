#include "widgets/richText.hpp"

#include "core/app.hpp"
#include "utils.hpp"
#include "widgets/layoutBuilder.hpp"
#include "widgets/stack.hpp"
#include "widgets/text.hpp"
#include <cmath>
#include <limits>


namespace squi {
	namespace {
		[[nodiscard]] std::shared_ptr<FontStore::Font> resolveFont(const RichText::FontVariant &font) {
			return std::visit(
				utils::overloaded{
					[](const FontProvider &provider) {
						return FontStore::getFont(provider);
					},
					[](const std::shared_ptr<FontStore::Font> &font) {
						return font;
					},
				},
				font
			);
		}

		[[nodiscard]] bool fontVariantEqual(const RichText::FontVariant &a, const RichText::FontVariant &b) {
			if (a.index() != b.index()) return false;
			if (std::holds_alternative<FontProvider>(a)) {
				return std::get<FontProvider>(a).key == std::get<FontProvider>(b).key;
			}
			return std::get<std::shared_ptr<FontStore::Font>>(a) == std::get<std::shared_ptr<FontStore::Font>>(b);
		}

		[[nodiscard]] bool spansEqual(const std::vector<RichText::Span> &a, const std::vector<RichText::Span> &b) {
			if (a.size() != b.size()) return false;
			for (size_t i = 0; i < a.size(); ++i) {
				const auto &spanA = a.at(i);
				const auto &spanB = b.at(i);
				if (spanA.index() != spanB.index()) return false;
				if (std::holds_alternative<std::string>(spanA)) {
					if (std::get<std::string>(spanA) != std::get<std::string>(spanB)) return false;
					continue;
				}
				const auto &styleA = std::get<RichText::Style>(spanA);
				const auto &styleB = std::get<RichText::Style>(spanB);
				return styleA == styleB;
			}
			return true;
		}
	}// namespace

	RichTextLayout richTextLayout(
		const std::vector<RichText::Span> &spans,
		float defaultSize,
		std::shared_ptr<FontStore::Font> defaultFont,
		Color defaultColor,
		std::optional<float> maxWidth,
		float scale
	) {
		struct Entry {
			std::string text;
			std::shared_ptr<FontStore::Font> font;
			float size;
			Color color;
		};
		std::vector<Entry> entries;
		entries.reserve(spans.size());
		for (const auto &span: spans) {
			std::visit(
				utils::overloaded{
					[&](const std::string &text) {
						if (text.empty()) return;
						entries.push_back({.text = text, .font = defaultFont, .size = defaultSize, .color = defaultColor});
					},
					[&](const RichText::Style &style) {
						if (style.text.empty()) return;
						entries.push_back({
							.text = style.text,
							.font = style.font.has_value() ? resolveFont(*style.font) : defaultFont,
							.size = style.fontSize.value_or(defaultSize),
							.color = style.color.value_or(defaultColor),
						});
					},
				},
				span
			);
		}

		RichTextLayout ret;
		if (entries.empty()) return ret;

		float blockAscender = -std::numeric_limits<float>::infinity();
		float blockDescender = std::numeric_limits<float>::infinity();
		for (const auto &run: entries) {
			const auto metrics = run.font->getFontMetrics(run.size, scale);
			blockAscender = std::max(blockAscender, metrics.ascender);
			blockDescender = std::min(blockDescender, metrics.descender);
		}
		const float blockLineHeight = blockAscender - blockDescender;

		float cursorX = 0.f;
		int lineIndex = 0;
		ret.segments.reserve(entries.size());
		for (const auto &run: entries) {
			const auto metrics = run.font->getFontMetrics(run.size, scale);
			const vec2 origin{cursorX, static_cast<float>(lineIndex) * blockLineHeight + (blockAscender - metrics.ascender)};
			auto layout = run.font->textLayout(run.text, run.size, origin, blockLineHeight, maxWidth, scale);
			for (auto &quadVec: layout.quads) {
				for (auto &quad: quadVec) {
					quad.setColor(run.color);
				}
			}

			uint32_t lastLine = 0;
			for (const auto &glyph: layout.glyphs) {
				lastLine = std::max(lastLine, glyph.lineIndex);
			}
			float lastWidth = 0.f;
			for (const auto &glyph: layout.glyphs) {
				if (glyph.lineIndex == lastLine) lastWidth = std::max(lastWidth, glyph.x + glyph.advance);
			}
			lineIndex += static_cast<int>(lastLine);
			cursorX = lastWidth;

			ret.segments.push_back({
				.layout = std::make_shared<const TextLayout>(std::move(layout)),
				.font = run.font,
				.fontSize = run.size,
			});
		}

		for (const auto &segment: ret.segments) {
			ret.size.x = std::max(ret.size.x, segment.layout->widestLine);
			ret.size.y = std::max(ret.size.y, segment.layout->totalHeight);
		}
		return ret;
	}

	void RichText::State::widgetUpdated() {
		const auto *w = this->widget;
		if (!spansEqual(w->text, lastText)
			|| w->fontSize != lastFontSize
			|| w->lineWrap != lastLineWrap
			|| !fontVariantEqual(w->font, lastFont)
			|| w->color != lastColor) {
			layoutDirty = true;
		}
	}

	void RichText::State::initState() {
		onScalingChanged = element->getApp()->surface.onScaleChange.observe([this]() {
			layoutDirty = true;
			element->markNeedsRebuild();
		});
	}


	Child RichText::State::build(const Element &) {
		return LayoutBuilder{
			.builder = [this](const BoxConstraints &constraints) -> Child {
				const auto *w = this->widget;
				const auto scale = element->getApp()->surface.scale;
				const auto maxWidth = w->lineWrap && std::isfinite(constraints.maxWidth) ? std::optional<float>(constraints.maxWidth) : std::nullopt;

				if (layoutDirty || (maxWidth.has_value() && cachedLayoutWidth != maxWidth)) {
					cachedLayout = std::make_shared<const RichTextLayout>(richTextLayout(w->text, w->fontSize, resolveFont(w->font), w->color, maxWidth, scale));
					cachedLayoutWidth = maxWidth;
					layoutDirty = false;
					lastText = w->text;
					lastFontSize = w->fontSize;
					lastLineWrap = w->lineWrap;
					lastFont = w->font;
					lastColor = w->color;
				}

				std::vector<Child> children;
				children.reserve(cachedLayout->segments.size());
				for (const auto &segment: cachedLayout->segments) {
					children.push_back(Text{
						.text = segment.layout,
						.fontSize = segment.fontSize,
						.font = segment.font,
					});
				}
				return Stack{
					.widget{
						.width = Size::Wrap,
						.height = Size::Wrap,
					},
					.children = std::move(children),
				};
			},
		};
	}
}// namespace squi
