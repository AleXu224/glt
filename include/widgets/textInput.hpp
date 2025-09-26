#pragma once

#include "core/core.hpp"
#include "fontStore.hpp"
#include "widgets/gestureDetector.hpp"
#include <GLFW/glfw3.h>


namespace squi {
	struct TextInput : StatefulWidget {
		struct Controller {
			std::string text;
			int64_t cursor = 0;
			std::optional<int64_t> selectionStart;
		};

		// Args
		Key key;
		Args widget;
		std::shared_ptr<Controller> controller;
		bool active = false;

		struct State : WidgetState<TextInput> {
			std::shared_ptr<Controller> controller;
			float scroll = 0.f;

			std::shared_ptr<FontStore::Font> font = FontStore::getFont(FontStore::defaultFont);

			void initState() override {
				controller = std::make_shared<Controller>();
				if (!controller) {
					controller = std::make_shared<Controller>();
				}
			}

			void widgetUpdated() override {
				if (widget->controller)
					controller = widget->controller;
			}

			void clampCursors();

			[[nodiscard]] int64_t getSelectionMin() const;
			[[nodiscard]] int64_t getSelectionMax() const;

			void setText(const std::string &text);
			void clearSelection();

			void handleTextInput(const Gesture::State &state);

			[[nodiscard]] uint64_t getPrevWordStart() const;
			[[nodiscard]] uint64_t getNextWordStart() const;

			void handleBackspace(const Gesture::State &state);
			void handleDelete(const Gesture::State &state);
			void handleLeftArrow(const Gesture::State &state);
			void handleRightArrow(const Gesture::State &state);
			void handleHome(const Gesture::State &state);
			void handleEnd(const Gesture::State &state);
			void handleEscape(const Gesture::State &state) const;
			void handleSelectAll(const Gesture::State &state);
			void handleCopy(const Gesture::State &state) const;
			void handleCut(const Gesture::State &state);
			void handlePaste(const Gesture::State &state);

			[[nodiscard]] Child getSelectionBox(uint32_t widthToStart) const;
			[[nodiscard]] Child getCursorBox(uint32_t widthToCursor) const;

			Child build(const Element &) override;
		};
	};
}// namespace squi