#pragma once

#include "core/core.hpp"
#include "fontStore.hpp"
#include "observer.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/misc/scrollViewData.hpp"
#include <GLFW/glfw3.h>


namespace squi {
	struct TextInput : StatefulWidget {
		struct Controller {
		private:
			struct ControlBlock {
				Observable<const std::string &> textObservable{};
				std::optional<std::string> initialText{};
				std::string *textPtr = nullptr;
			};
			std::shared_ptr<ControlBlock> controlBlock = std::make_shared<ControlBlock>();

			void mount(std::string &text) {
				controlBlock->textPtr = &text;
			}

		public:
			Controller() = default;
			Controller(const std::string &initial) {
				controlBlock->initialText = initial;
			}

			[[nodiscard]] Observer<const std::string &> getTextObserver(const std::function<void(const std::string &)> &callback) const {
				return controlBlock->textObservable.observe(callback);
			}

			void setText(const std::string &text) const {
				controlBlock->textObservable.notify(text);
			}

			[[nodiscard]] const std::string &getText() const {
				assert(controlBlock->textPtr);
				return *controlBlock->textPtr;
			}

			friend TextInput;
		};

		// Args
		Key key;
		Args widget;
		Controller controller{};
		std::function<void(const std::string &)> onTextChanged;
		bool active = false;

		struct State : WidgetState<TextInput> {
			ScrollViewData cachedScrollData{};
			std::shared_ptr<ScrollViewData> scrollController = std::make_shared<ScrollViewData>();
			Controller controller{};
			Observer<const std::string &> textObserver{};
			std::string text;
			int64_t cursor = 0;
			std::optional<int64_t> selectionStart;
			float scroll = 0.f;

			std::shared_ptr<FontStore::Font> font = FontStore::getFont(FontStore::defaultFont);

			void initState() override {
				controller = widget->controller;
				controller.mount(text);
				text = controller.controlBlock->initialText.value_or("");
				textObserver = controller.getTextObserver([this](const std::string &newText) {
					setText(newText);
				});
			}

			void widgetUpdated() override {}

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
			void handleEscape(const Gesture::State &state);
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