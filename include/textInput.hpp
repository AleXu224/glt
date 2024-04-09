#pragma once

#include "color.hpp"
#include "fontStore.hpp"
#include "observer.hpp"
#include "widget.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>


namespace squi {
	struct TextInput {
		// Args
		Widget::Args widget{};
		float fontSize = 14.0f;
		std::variant<std::string_view, std::shared_ptr<FontStore::Font>> font = FontStore::defaultFont;
		std::string_view text{};
		std::function<void(std::string_view)> onTextChanged{};
		Color color{0xFFFFFFFF};
		struct Controller {
			Observable<bool> setActive{};
			VoidObservable selectAll{};
		};
		Controller controller{};

		class Impl : public Widget {
			// Data
			int64_t cursor{};
			float scroll{};
			Child textWidget;
			Child selectionWidget;
			Child selectionTextWidget;
			Child cursorWidget;
			std::function<void(std::string_view)> onTextChanged;
			std::optional<int64_t> selectionStart{};

			Observer<bool> setActiveObs{};
			VoidObserver selectAllObs{};

			bool active = false;

		public:
			explicit Impl(const TextInput &args);

			void onUpdate() final;
			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) final;
			void arrangeChildren(vec2 &pos) final;

			void setActive(bool active);

			void drawChildren() final;

			void clampCursors();
			void clearSelection();
			void handleMouseInput();
			void updateSelection();

			std::string_view getText();
			void setText(std::string_view text);
			void setColor(const Color &newColor);
			int64_t getSelectionMin();
			int64_t getSelectionMax();
		};

		operator Child() const;
	};
}// namespace squi