#pragma once

#include "core/core.hpp"
#include "fontStore.hpp"
#include "observer.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/misc/multilineTextEditor.hpp"
#include "widgets/misc/scrollViewData.hpp"
#include "widgets/textInput.hpp"


namespace squi {
	struct TextArea : StatefulWidget {
		Key key;
		Args widget;
		TextInput::Controller controller{};
		std::function<void(const std::string &)> onTextChanged;
		bool active = false;

		struct State : WidgetState<TextArea> {
			MultilineTextEditor buffer;

			TextInput::Controller controller{};
			Observer<const std::string &> textObserver{};

			std::shared_ptr<FontStore::Font> font = FontStore::getFont(FontStore::defaultFont);

			float scrollY = 0.f;
			float scrollX = 0.f;
			std::shared_ptr<ScrollViewData> verticalScrollController = std::make_shared<ScrollViewData>();
			std::shared_ptr<ScrollViewData> horizontalScrollController = std::make_shared<ScrollViewData>();
			ScrollViewData cachedVerticalScrollData{};
			ScrollViewData cachedHorizontalScrollData{};
			bool manuallyScrolling = false;

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

			void initState() override;
			void widgetUpdated() override;

			int64_t indexFromPos(float x, float y) const;

			void handleMousePress(const Gesture::State &state);
			void handleMouseDrag(const Gesture::State &state);
			float getRelativeCursorX(const Gesture::State &state) const;
			float getRelativeCursorY(const Gesture::State &state) const;

			Child buildSelectionBoxes() const;
			Child buildCursorBox() const;
			Child build(const Element &) override;
		};
	};
}// namespace squi
