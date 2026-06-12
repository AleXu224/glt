#include "./multilineTextEditor.hpp"

namespace squi {

	void MultilineTextEditor::handleEnter(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::enter)) {
			clearSelection();
			text->insert(cursor, "\n");
			++cursor;
			onTextChanged(*text);
		}
	}
	void MultilineTextEditor::handleUpArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::up)) {
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor > 0) {
				if (!selectionStart.has_value()) {
					selectionStart = cursor;
				}
			} else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value()) {
				selectionStart = std::nullopt;
			}
			if (!cachedLayoutPtr) return;

			uint32_t currentLine = cachedLayoutPtr->lineForOffset(cursor);
			if (currentLine == 0) {
				cursor = 0;
				return;
			}

			float cursorX = cachedLayoutPtr->xForOffset(cursor);
			uint32_t targetLine = currentLine - 1;

			auto it = std::lower_bound(cachedLayoutPtr->glyphs.begin(), cachedLayoutPtr->glyphs.end(), targetLine, [](const TextLayout::Glyph &g, uint32_t line) {
				return g.lineIndex < line || (g.isNewline() && g.lineIndex == line);
			});
			if (it == cachedLayoutPtr->glyphs.end() || it->lineIndex != targetLine) {
				if (targetLine == 0) cursor = 0;
				else if (targetLine <= cachedLayoutPtr->newlineOffsets.size())
					cursor = cachedLayoutPtr->newlineOffsets[targetLine - 1] + 1;
				return;
			}

			auto end = it;
			while (end != cachedLayoutPtr->glyphs.end() && end->lineIndex == targetLine) ++end;

			auto best = std::lower_bound(it, end, cursorX, [](const TextLayout::Glyph &g, float val) {
				return (g.x + g.advance * 0.5f) < val;
			});
			if (best == end) --best;

			cursor = best->byteOffset;
		}
	}
	void MultilineTextEditor::handleDownArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::down)) {
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor > 0) {
				if (!selectionStart.has_value()) {
					selectionStart = cursor;
				}
			} else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value()) {
				selectionStart = std::nullopt;
			}
			if (!cachedLayoutPtr) return;

			uint32_t currentLine = cachedLayoutPtr->lineForOffset(cursor);
			if (currentLine == cachedLayoutPtr->quads.size() - 1) {
				cursor = cachedLayoutPtr->glyphs.back().byteOffset + 1;
				return;
			}
			float cursorX = cachedLayoutPtr->xForOffset(cursor);
			uint32_t targetLine = currentLine + 1;

			auto it = std::lower_bound(cachedLayoutPtr->glyphs.begin(), cachedLayoutPtr->glyphs.end(), targetLine, [](const TextLayout::Glyph &g, uint32_t line) {
				return g.lineIndex < line || (g.isNewline() && g.lineIndex == line);
			});
			if (it == cachedLayoutPtr->glyphs.end() || it->lineIndex != targetLine) {
				cursor = cachedLayoutPtr->glyphs.back().byteOffset + 1;
				return;
			}

			auto end = it;
			while (end != cachedLayoutPtr->glyphs.end() && end->lineIndex == targetLine) ++end;

			auto best = std::lower_bound(it, end, cursorX, [](const TextLayout::Glyph &g, float val) {
				return (g.x + g.advance * 0.5f) < val;
			});
			if (best == end) --best;

			cursor = best->byteOffset;
			if (targetLine == cachedLayoutPtr->quads.size()) cursor++;
		}
	}
	void MultilineTextEditor::handleLeftArrow(const Gesture::State &state) {
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
	void MultilineTextEditor::handleRightArrow(const Gesture::State &state) {
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
	void MultilineTextEditor::handleHome(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::home)) {
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor > 0 && !selectionStart.has_value())
				selectionStart = cursor;
			else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value())
				selectionStart = std::nullopt;

			if (key->mods & static_cast<int>(GestureMod::control)) {
				cursor = 0;
			} else if (cachedLayoutPtr) {
				uint32_t line = cachedLayoutPtr->lineForOffset(cursor);
				if (line == 0) {
					cursor = 0;
				} else if (line <= cachedLayoutPtr->newlineOffsets.size()) {
					cursor = cachedLayoutPtr->newlineOffsets[line - 1];
				} else {
					cursor = 0;
				}
			} else {
				cursor = 0;
			}
		}
	}
	void MultilineTextEditor::handleEnd(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::end)) {
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor < static_cast<int64_t>(text->size()) && !selectionStart.has_value())
				selectionStart = cursor;
			else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value())
				selectionStart = std::nullopt;

			if (key->mods & static_cast<int>(GestureMod::control)) {
				cursor = static_cast<int64_t>(text->size());
			} else if (cachedLayoutPtr) {
				uint32_t line = cachedLayoutPtr->lineForOffset(cursor);
				if (line < cachedLayoutPtr->newlineOffsets.size()) {
					cursor = cachedLayoutPtr->newlineOffsets[line] - 1;
				} else {
					cursor = static_cast<int64_t>(text->size());
				}
			} else {
				cursor = static_cast<int64_t>(text->size());
			}
		}
	}


}// namespace squi