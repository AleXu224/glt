#pragma once

#include "core/core.hpp"
#include "fontStore.hpp"
#include "text/provider.hpp"
#include <optional>
#include <variant>
#include <vector>


namespace squi {
	struct RichTextLayout;

	struct RichText : StatefulWidget {
		using FontVariant = std::variant<FontProvider, std::shared_ptr<FontStore::Font>>;

		struct Style {
			std::optional<float> fontSize;
			std::optional<Color> color;
			std::optional<FontVariant> font;
			std::string text;

			bool operator==(const Style &other) const {
				auto equal = text == other.text
						  && fontSize == other.fontSize
						  && color == other.color
						  && font.has_value() == other.font.has_value();
				if (!equal) return false;
				if (font.has_value()) {
					if (font->index() != other.font->index()) return false;
					if (std::holds_alternative<FontProvider>(*font)) {
						return std::get<FontProvider>(*font).key == std::get<FontProvider>(*other.font).key;
					}
					return std::get<std::shared_ptr<FontStore::Font>>(*font) == std::get<std::shared_ptr<FontStore::Font>>(*other.font);
				}
				return true;
			}
		};
		using Span = std::variant<std::string, Style>;

		Key key;
		Args widget;
		std::vector<Span> text;
		float fontSize{14.f};
		bool lineWrap{false};
		FontVariant font = FontStore::defaultFont;
		Color color = Color::white;

		struct State : WidgetState<RichText> {
			std::shared_ptr<const RichTextLayout> cachedLayout;
			std::optional<float> cachedLayoutWidth;
			VoidObserver onScalingChanged{};
			std::vector<Span> lastText;
			float lastFontSize{14.f};
			bool lastLineWrap{false};
			FontVariant lastFont = FontStore::defaultFont;
			Color lastColor = Color::white;
			bool layoutDirty = true;

			void widgetUpdated() override;
			void initState() override;
			Child build(const Element &) override;
		};
	};

	struct RichTextLayout {
		struct Segment {
			std::shared_ptr<const TextLayout> layout;
			std::shared_ptr<FontStore::Font> font;
			float fontSize;
		};
		std::vector<Segment> segments;
		vec2 size;
	};

	[[nodiscard]] RichTextLayout richTextLayout(
		const std::vector<RichText::Span> &spans,
		float defaultSize,
		std::shared_ptr<FontStore::Font> defaultFont,
		Color defaultColor,
		std::optional<float> maxWidth,
		float scale
	);
}// namespace squi
