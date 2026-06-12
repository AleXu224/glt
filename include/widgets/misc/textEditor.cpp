#include "widgets/misc/textEditor.hpp"

#include "widgets/gestureDetector.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>

namespace squi {
	void TextEditor::regenerateLayout(const std::shared_ptr<FontStore::Font> &font) {
		cachedLayoutPtr = std::make_shared<const TextLayout>(font->textLayout(*text, 14.f));
	}

	void TextEditor::clampCursors() {
		cursor = std::clamp(cursor, static_cast<int64_t>(0), static_cast<int64_t>(text->size()));
		if (selectionStart.has_value()) {
			auto clamped = std::clamp(selectionStart.value_or(0), static_cast<int64_t>(0), static_cast<int64_t>(text->size()));
			if (cursor == clamped) {
				selectionStart = std::nullopt;
			} else {
				selectionStart = clamped;
			}
		}
	}

	int64_t TextEditor::getSelectionMin() const {
		if (!selectionStart.has_value()) return cursor;
		return std::min(selectionStart.value(), cursor);
	}

	int64_t TextEditor::getSelectionMax() const {
		if (!selectionStart.has_value()) return cursor;
		return std::max(selectionStart.value(), cursor);
	}

	void TextEditor::setText(const std::string &newText) {
		if (*text == newText) return;
		*text = newText;
		onTextChanged(*text);
	}

	void TextEditor::clearSelection() {
		clampCursors();
		if (!selectionStart.has_value()) return;
		const auto min = getSelectionMin();
		const auto max = getSelectionMax();

		text->erase(min, max - min);
		cursor = min;
		selectionStart = std::nullopt;
		onTextChanged(*text);
	}

	void TextEditor::handleTextInput(const std::string &g_textInput) {
		if (g_textInput.empty()) return;
		clearSelection();
		text->insert(cursor, g_textInput);
		cursor += static_cast<int64_t>(g_textInput.size());
		onTextChanged(*text);
	}

	uint64_t TextEditor::getPrevWordStart(int64_t position) const {
		if (position <= 0) return 0;
		auto it = text->begin() + (position - 1);
		if (it != text->end() && std::isspace(*it)) {
			it = std::find_if(std::make_reverse_iterator(text->begin() + position), text->rend(), [](char c) {
					 return !std::isspace(static_cast<unsigned char>(c));
				 }).base();
		}
		it = std::find_if(std::make_reverse_iterator(it), text->rend(), isspace).base();
		return std::distance(text->begin(), it);
	}

	uint64_t TextEditor::getNextWordStart(int64_t position) const {
		if (position >= static_cast<int64_t>(text->size())) return text->size();
		auto it = text->begin() + position;
		if (it != text->end() && std::isspace(*it)) {
			it = std::find_if(it, text->end(), [](char c) {
				return !std::isspace(static_cast<unsigned char>(c));
			});
		}
		it = std::find_if(it, text->end(), isspace);
		return std::distance(text->begin(), it);
	}

	std::pair<int64_t, int64_t> TextEditor::getWordRange(int64_t index) const {
		if (text->empty()) return {0, 0};
		auto it = text->begin() + std::clamp(index, static_cast<int64_t>(0), static_cast<int64_t>(text->size() - 1));

		bool targetIsSpace = std::isspace(*it);

		auto s = it;
		while (s != text->begin() && std::isspace(*(s - 1)) == targetIsSpace)
			--s;

		auto e = it;
		while (e != text->end() && std::isspace(*e) == targetIsSpace)
			++e;

		return {std::distance(text->begin(), s), std::distance(text->begin(), e)};
	}

	void TextEditor::handleBackspace(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::backspace)) {
			if (selectionStart.has_value()) {
				clearSelection();
				return;
			}
			const auto &keyState = key.value();
			if (keyState.mods & static_cast<int>(GestureMod::control) && cursor > 0) {
				auto pos = getPrevWordStart(cursor);
				text->erase(pos, cursor - pos);
				cursor = static_cast<int64_t>(pos);
				onTextChanged(*text);
			} else if (cursor > 0) {
				bool isCursorAtEnd = cursor == static_cast<int64_t>(text->size());
				text->erase(cursor - 1, 1);
				if (!isCursorAtEnd) {
					--cursor;
				}
				onTextChanged(*text);
			}
		}
	}

	void TextEditor::handleDelete(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::del)) {
			if (selectionStart.has_value()) {
				clearSelection();
				return;
			}
			const auto &keyState = key.value();
			if (keyState.mods & static_cast<int>(GestureMod::control) && cursor < static_cast<int64_t>(text->size())) {
				auto pos = getNextWordStart(cursor);
				text->erase(cursor, pos - cursor);
				onTextChanged(*text);
			} else if (cursor < static_cast<int64_t>(text->size())) {
				text->erase(cursor, 1);
				onTextChanged(*text);
			}
		}
	}

	void TextEditor::handleLeftArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::left)) {
			bool removedSelection = false;
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor > 0) {
				if (!selectionStart.has_value()) {
					selectionStart = cursor;
				}
			} else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value()) {
				clampCursors();
				cursor = getSelectionMin();
				selectionStart = std::nullopt;
				removedSelection = true;
			}

			if (cursor > 0 && !removedSelection) {
				if (key->mods & static_cast<int>(GestureMod::control)) {
					auto pos = getPrevWordStart(cursor);
					cursor = static_cast<int64_t>(pos);
				} else {
					--cursor;
				}
			}
		}
	}

	void TextEditor::handleRightArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::right)) {
			bool removedSelection = false;
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor < static_cast<int64_t>(text->size())) {
				if (!selectionStart.has_value()) {
					selectionStart = cursor;
				}
			} else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value()) {
				clampCursors();
				cursor = getSelectionMax();
				selectionStart = std::nullopt;
				removedSelection = true;
			}

			if (cursor < static_cast<int64_t>(text->size()) && !removedSelection) {
				if (key->mods & static_cast<int>(GestureMod::control)) {
					auto pos = getNextWordStart(cursor);
					cursor = static_cast<int64_t>(pos);
				} else {
					++cursor;
				}
			}
		}
	}

	void TextEditor::handleHome(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::home)) {
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor > 0 && !selectionStart.has_value())
				selectionStart = cursor;
			else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value())
				selectionStart = std::nullopt;

			cursor = std::min<int64_t>(cursor, 0);
		}
	}

	void TextEditor::handleEnd(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::end)) {
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor < static_cast<int64_t>(text->size()) && !selectionStart.has_value())
				selectionStart = cursor;
			else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value())
				selectionStart = std::nullopt;

			cursor = std::max(cursor, static_cast<int64_t>(text->size()));
		}
	}

	void TextEditor::handleCopy(const Gesture::State &state) const {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::c); key && key->mods & static_cast<int>(GestureMod::control) && selectionStart.has_value()) {
			auto textToCopy = text->substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
			glfwSetClipboardString(nullptr, textToCopy.c_str());
		}
	}

	void TextEditor::handleCut(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::x); key && key->mods & static_cast<int>(GestureMod::control) && selectionStart.has_value()) {
			auto textToCopy = text->substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
			glfwSetClipboardString(nullptr, textToCopy.c_str());
			clearSelection();
		}
	}

	void TextEditor::handlePaste(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::v); key && key->mods & static_cast<int>(GestureMod::control)) {
			const auto *const clipboardText = glfwGetClipboardString(nullptr);
			if (!clipboardText) return;
			clearSelection();
			const auto clipboardString = std::string_view(clipboardText);
			text->insert(cursor, clipboardString);
			cursor += static_cast<int64_t>(clipboardString.size());
			onTextChanged(*text);
		}
	}

	void TextEditor::handleSelectAll(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::a); key && key->mods & static_cast<int>(GestureMod::control)) {
			selectionStart = 0;
			cursor = static_cast<int64_t>(text->size());
		}
	}

	void TextEditor::handleEscape(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::escape)) {
			selectionStart = std::nullopt;
		}
	}
}// namespace squi
