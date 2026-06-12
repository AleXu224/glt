#pragma once

#include "core/core.hpp"
#include "fontStore.hpp"
#include "text/provider.hpp"
#include <variant>


namespace squi {
	struct TextData;

	using TextVariant = std::variant<std::string, std::shared_ptr<const TextLayout>>;

	struct Text : RenderObjectWidget {
		Key key;
		Args widget{};

		TextVariant text;
		float fontSize{14.0f};
		bool lineWrap{false};
		std::variant<FontProvider, std::shared_ptr<FontStore::Font>> font = FontStore::defaultFont;
		Color color = Color::white;

		struct TextRenderObject : RenderObject {
			std::string text{};
			std::shared_ptr<const TextLayout> precomputedLayout;
			float fontSize{14.0f};
			bool lineWrap{false};
			std::shared_ptr<FontStore::Font> font{};
			Color color = Color::white;
			vec2 textSize{};
			vec2 lastAvailableSpace{0};
			vec2 lastPos{0};

			bool forceRegen = false;
			std::unique_ptr<TextData> data;

			TextRenderObject();

			void init() override;
			vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
			void afterSizeCalculated() override;

			void positionQuadsAt(const vec2 &pos);
			void positionContentAt(const Rect &newBounds) override;

			void drawContent() override;
		};

		static RenderObjectPtr createRenderObject();

		void updateRenderObject(RenderObject *renderObject) const;

		[[nodiscard]] Args getArgs() const {
			auto ret = widget;
			ret.width = Size::Wrap;
			ret.height = Size::Wrap;
			return ret;
		}
	};
}// namespace squi
