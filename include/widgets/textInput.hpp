#pragma once

#include "core/core.hpp"
#include "fontStore.hpp"
#include "observer.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/misc/scrollViewData.hpp"
#include "widgets/misc/textEditor.hpp"


namespace squi {
	struct TextArea;
	struct TextInput : StatefulWidget {
		struct Controller {
		private:
			struct ControlBlock {
				Observable<const std::string &> textObservable{};
				std::string text{};
			};
			std::shared_ptr<ControlBlock> controlBlock = std::make_shared<ControlBlock>();

		public:
			Controller(const std::string &initial = "") {
				controlBlock->text = initial;
			}

			[[nodiscard]] Observer<const std::string &> getTextObserver(const std::function<void(const std::string &)> &callback) const {
				return controlBlock->textObservable.observe(callback);
			}

			void setText(const std::string &text) const {
				if (controlBlock->text == text) return;
				controlBlock->text = text;
				controlBlock->textObservable.notify(controlBlock->text);
			}

			[[nodiscard]] const std::string &getText() const {
				return controlBlock->text;
			}

			bool operator==(const Controller &other) const {
				return controlBlock == other.controlBlock;
			}

			void notifyTextChanged() const {
				controlBlock->textObservable.notify(controlBlock->text);
			}

			friend TextInput;
			friend TextArea;
		};

		// Args
		Key key;
		Args widget;
		Controller controller{};
		std::function<void(const std::string &)> onTextChanged;
		bool active = false;

		struct State : WidgetState<TextInput> {
			TextEditor buffer;
			Controller controller{};
			Observer<const std::string &> textObserver{};

			ScrollViewData cachedScrollData{};
			std::shared_ptr<ScrollViewData> scrollController = std::make_shared<ScrollViewData>();
			bool manuallyScrolling = false;
			float scroll = 0.f;

			std::chrono::steady_clock::time_point lastClickTime = std::chrono::steady_clock::now();
			int clickCount = 0;
			int64_t lastClickedIndex = 0;
			enum class DragType {
				Char,
				Word,
				Line,
			};
			DragType dragType = DragType::Char;
			int64_t pivot = 0;
			std::pair<int64_t, int64_t> pivotRange{0, 0};

			std::shared_ptr<FontStore::Font> font = FontStore::getFont(FontStore::defaultFont);

			void initState() override {
				controller = widget->controller;
				buffer.text = &controller.controlBlock->text;
				buffer.onTextChanged = [this](const std::string &text) {
					buffer.regenerateLayout(font);
					controller.notifyTextChanged();
					if (widget->onTextChanged) widget->onTextChanged(text);
				};
				textObserver = controller.getTextObserver([this](const std::string &newText) {
					buffer.regenerateLayout(font);
					buffer.clampCursors();
				});
				buffer.regenerateLayout(font);
			}

			void widgetUpdated() override {
				if (controller == widget->controller) return;
				controller = widget->controller;
				buffer.text = &controller.controlBlock->text;
				buffer.onTextChanged = [this](const std::string &text) {
					buffer.regenerateLayout(font);
					controller.notifyTextChanged();
					if (widget->onTextChanged) widget->onTextChanged(text);
				};
				textObserver = controller.getTextObserver([this](const std::string &newText) {
					buffer.regenerateLayout(font);
					buffer.clampCursors();
				});
			}

			void handleMousePress(const Gesture::State &state);
			void handleMouseDrag(const Gesture::State &state);
			[[nodiscard]] int64_t indexFromPos(float x) const;
			[[nodiscard]] std::pair<int64_t, int64_t> getWordRange(int64_t index) const;
			[[nodiscard]] float getRelativeCursorX(const Gesture::State &state) const;

			[[nodiscard]] Child getSelectionBox(uint32_t widthToStart) const;
			[[nodiscard]] Child getCursorBox(uint32_t widthToCursor) const;

			Child build(const Element &) override;
		};
	};
}// namespace squi
