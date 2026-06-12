#pragma once

#include "fontStore.hpp"
#include "widgets/gestureDetector.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace squi {
	struct TextEditor {
		std::string *text;
		std::shared_ptr<const TextLayout> cachedLayoutPtr;
		int64_t cursor = 0;
		std::optional<int64_t> selectionStart;

		std::function<void(const std::string &)> onTextChanged;

		void regenerateLayout(const std::shared_ptr<FontStore::Font> &font);

		void clampCursors();

		[[nodiscard]] int64_t getSelectionMin() const;
		[[nodiscard]] int64_t getSelectionMax() const;

		void setText(const std::string &newText);
		void clearSelection();

		void handleTextInput(const std::string &g_textInput);

		[[nodiscard]] uint64_t getPrevWordStart(int64_t position) const;
		[[nodiscard]] uint64_t getNextWordStart(int64_t position) const;
		[[nodiscard]] std::pair<int64_t, int64_t> getWordRange(int64_t index) const;

		virtual void handleEnter(const Gesture::State &state) {}

		virtual void handleBackspace(const Gesture::State &state);
		virtual void handleDelete(const Gesture::State &state);

		virtual void handleUpArrow(const Gesture::State &state) {}
		virtual void handleDownArrow(const Gesture::State &state) {}
		virtual void handleLeftArrow(const Gesture::State &state);
		virtual void handleRightArrow(const Gesture::State &state);

		virtual void handleHome(const Gesture::State &state);
		virtual void handleEnd(const Gesture::State &state);

		virtual void handleCopy(const Gesture::State &state) const;
		virtual void handleCut(const Gesture::State &state);
		virtual void handlePaste(const Gesture::State &state);

		virtual void handleSelectAll(const Gesture::State &state);
		virtual void handleEscape(const Gesture::State &state);
	};
}// namespace squi
