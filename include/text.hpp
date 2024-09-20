#pragma once

#include "color.hpp"
#include "fontStore.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include <memory>
#include <string_view>


namespace squi {
	struct TextData;
	
	struct Text {
		// Args
		Widget::Args widget{};
		std::string_view text;
		float fontSize{14.0f};
		bool lineWrap{false};
		std::variant<FontProvider, std::shared_ptr<FontStore::Font>> font = FontStore::defaultFont;
		Color color{0xFFFFFFFF};

		class Impl : public Widget {
			// Data
			float lastX{0};
			float lastY{0};
			float lastAvailableSpace{0};
			std::string text;
			float fontSize;
			bool lineWrap;
			bool forceRegen = false;
			std::shared_ptr<FontStore::Font> font{};
			Color color;
			vec2 textSize;

			std::unique_ptr<TextData> data;

		public:
			Impl(const Impl &) = delete;
			Impl(Impl &&) = delete;
			Impl &operator=(const Impl &) = delete;
			Impl &operator=(Impl &&) = delete;
			Impl(const Text &args);
			~Impl() override;

			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) final;
			void postLayout(vec2 &size) override;
			void onArrange(vec2 &pos) final;

			void updateSize();

			void onDraw() final;

			void setText(const std::string_view &text);
			void setColor(const Color &newColor);

			[[nodiscard]] std::tuple<uint32_t, uint32_t> getTextSize(const std::string_view &text) const;

			[[nodiscard]] std::string_view getText() const;

			[[nodiscard]] const TextData &getData() const;

			[[nodiscard]] uint32_t getLineHeight() const;
		};

		operator Child() const {
			return std::make_shared<Impl>(*this);
		}
	};
}// namespace squi