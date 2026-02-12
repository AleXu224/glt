#include "widgets/textInput.hpp"

#include "core/app.hpp"
#include "offset.hpp"
#include "theme.hpp"
#include "widgets/box.hpp"
#include "widgets/scrollable.hpp"
#include "widgets/stack.hpp"
#include "widgets/text.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>

namespace squi {
	void TextInput::State::clampCursors() {
		auto newCursor = std::clamp(cursor, static_cast<int64_t>(0), static_cast<int64_t>(text.size()));
		if (newCursor != cursor) {
			setState([this, newCursor]() {
				cursor = newCursor;
			});
		}
		if (selectionStart.has_value()) {
			std::optional<int64_t> newSelectionStart = std::clamp(selectionStart.value_or(0), static_cast<int64_t>(0), static_cast<int64_t>(text.size()));
			if (cursor == selectionStart.value())
				newSelectionStart = std::nullopt;

			if (newSelectionStart != selectionStart) {
				setState([this, newSelectionStart]() {
					selectionStart = newSelectionStart;
				});
			}
		}
	}

	int64_t TextInput::State::getSelectionMin() const {
		if (!selectionStart.has_value()) return cursor;
		return std::min(selectionStart.value(), cursor);
	}

	int64_t TextInput::State::getSelectionMax() const {
		if (!selectionStart.has_value()) return cursor;
		return std::max(selectionStart.value(), cursor);
	}

	void TextInput::State::setText(const std::string &newText) {
		if (text == newText) return;
		setState([this, newText]() {
			text = newText;
		});
		controller.setText(newText);
		if (widget->onTextChanged) widget->onTextChanged(text);
	}

	void TextInput::State::clearSelection() {
		clampCursors();
		if (!selectionStart.has_value()) return;
		const auto min = getSelectionMin();
		const auto max = getSelectionMax();

		std::string newText = std::format("{}{}", std::string(text.substr(0, min)), std::string(text.substr(max)));
		setText(newText);
		setState([&]() {
			cursor = min;
			selectionStart = std::nullopt;
		});
	}

	void TextInput::State::handleTextInput(const Gesture::State &state) {
		if (!state.inputState->g_textInput.empty()) {
			clearSelection();
			setText(std::format("{}{}{}", text.substr(0, cursor), state.inputState->g_textInput, text.substr(cursor)));
			setState([this, &state]() {
				cursor += static_cast<int64_t>(state.inputState->g_textInput.size());
			});
		}
	}

	uint64_t TextInput::State::getPrevWordStart(int64_t position) const {
		auto pos = text.find_last_of(' ', position - 1);
		if (pos == std::string::npos) pos = 0;
		return pos;
	}

	uint64_t TextInput::State::getNextWordStart(int64_t position) const {
		auto pos = text.find_first_of(' ', position + 1);
		if (pos == std::string::npos) pos = text.size();
		return pos;
	}

	void TextInput::State::handleBackspace(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::backspace)) {
			if (selectionStart.has_value()) {
				clearSelection();
				return;
			}
			const auto &keyState = key.value();
			if (keyState.mods & static_cast<int>(GestureMod::control) && cursor > 0) {
				auto pos = getPrevWordStart(cursor);

				setText(std::format("{}{}", text.substr(0, pos), text.substr(cursor)));
				setState([&]() {
					cursor = static_cast<int64_t>(pos);
				});
			} else if (cursor > 0) {
				bool isCursorAtEnd = cursor == static_cast<int64_t>(text.size());
				setText(std::format("{}{}", text.substr(0, cursor - 1), text.substr(cursor)));
				if (!isCursorAtEnd) {
					setState([&]() {
						--cursor;
					});
				}
			}
		}
	}

	void TextInput::State::handleDelete(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::del)) {
			if (selectionStart.has_value()) {
				clearSelection();
			} else {
				const auto &keyState = key.value();
				if (keyState.mods & static_cast<int>(GestureMod::control) && cursor < static_cast<int64_t>(text.size())) {
					auto pos = getNextWordStart(cursor);

					setText(std::format("{}{}", text.substr(0, cursor), text.substr(pos)));
				} else if (cursor < static_cast<int64_t>(text.size())) {
					setText(std::format("{}{}", text.substr(0, cursor), text.substr(cursor + 1)));
				}
			}
		}
	}

	void TextInput::State::handleLeftArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::left)) {
			bool removedSelection = false;
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor > 0) {
				if (!selectionStart.has_value()) setState([&]() {
					selectionStart = cursor;
				});
			} else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value()) {
				clampCursors();
				setState([&]() {
					cursor = getSelectionMin();
					selectionStart = std::nullopt;
				});
				removedSelection = true;
			}

			if (cursor > 0 && !removedSelection) {
				if (key->mods & static_cast<int>(GestureMod::control)) {
					auto pos = getPrevWordStart(cursor);

					setState([&]() {
						cursor = static_cast<int64_t>(pos);
					});
				} else {
					setState([&]() {
						--cursor;
					});
				}
			}
		}
	}

	void TextInput::State::handleRightArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::right)) {
			bool removedSelection = false;
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor < static_cast<int64_t>(text.size())) {
				if (!selectionStart.has_value()) setState([&]() {
					selectionStart = cursor;
				});
			} else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value()) {
				clampCursors();
				setState([&]() {
					cursor = getSelectionMax();
					selectionStart = std::nullopt;
				});
				removedSelection = true;
			}

			if (cursor < static_cast<int64_t>(text.size()) && !removedSelection) {
				if (key->mods & static_cast<int>(GestureMod::control)) {
					auto pos = getNextWordStart(cursor);

					setState([&]() {
						cursor = static_cast<int64_t>(pos);
					});
				} else {
					setState([&]() {
						++cursor;
					});
				}
			}
		}
	}

	void TextInput::State::handleHome(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::home)) {
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor > 0 && !selectionStart.has_value())
				setState([&]() {
					selectionStart = cursor;
				});
			else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value())
				setState([&]() {
					selectionStart = std::nullopt;
				});

			setState([&]() {
				cursor = std::min<int64_t>(cursor, 0);
			});
		}
	}

	void TextInput::State::handleEnd(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::end)) {
			if (key->mods & static_cast<int>(GestureMod::shift) && cursor < static_cast<int64_t>(text.size()) && !selectionStart.has_value())
				setState([&]() {
					selectionStart = cursor;
				});
			else if (!(key->mods & static_cast<int>(GestureMod::shift)) && selectionStart.has_value())
				setState([&]() {
					selectionStart = std::nullopt;
				});

			setState([&]() {
				cursor = std::max(cursor, static_cast<int64_t>(text.size()));
			});
		}
	}

	void TextInput::State::handleEscape(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::escape)) {
			selectionStart = std::nullopt;
		}
	}

	void TextInput::State::handleSelectAll(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::a); key && key->mods & static_cast<int>(GestureMod::control)) {
			setState([&]() {
				selectionStart = 0;
				cursor = static_cast<int64_t>(text.size());
			});
		}
	}

	void TextInput::State::handleCopy(const Gesture::State &state) const {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::c); key && key->mods & static_cast<int>(GestureMod::control) && selectionStart.has_value()) {
			auto textToCopy = text.substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
			glfwSetClipboardString(nullptr, textToCopy.c_str());
		}
	}

	void TextInput::State::handleCut(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::x); key && key->mods & static_cast<int>(GestureMod::control) && selectionStart.has_value()) {
			auto textToCopy = text.substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
			glfwSetClipboardString(nullptr, textToCopy.c_str());
			clearSelection();
		}
	}

	void TextInput::State::handlePaste(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GestureKey::v); key && key->mods & static_cast<int>(GestureMod::control)) {
			const auto *const clipboardText = glfwGetClipboardString(nullptr);
			if (!clipboardText) return;
			clearSelection();
			const auto clipboardString = std::string_view(clipboardText);
			const auto &textValue = text;
			setText(std::format("{}{}{}", textValue.substr(0, cursor), clipboardString, textValue.substr(cursor)));
			setState([&]() {
				cursor += static_cast<int64_t>(clipboardString.size());
			});
		}
	}

	int64_t TextInput::State::indexFromPos(float x) const {
		if (text.empty()) return 0;

		// Filter out UTF-8 continuation bytes
		std::vector<int64_t> indices;
		indices.reserve(text.size() + 1);
		for (size_t i = 0; i < text.size(); ++i) {
			unsigned char c = static_cast<unsigned char>(text[i]);
			if ((c & 0xC0) != 0x80) indices.push_back(i);
		}
		indices.push_back(text.size());

		auto it = std::lower_bound(indices.begin(), indices.end(), x, [&](int64_t idx, float val) {
			auto [w, h] = font->getTextSizeSafe(text.substr(0, idx), 14.f);
			return static_cast<float>(w) < val;
		});

		int64_t indexRight = (it == indices.end()) ? indices.back() : *it;
		if (it == indices.begin()) return indexRight;

		int64_t indexLeft = *(it - 1);
		float widthRight = std::get<0>(font->getTextSizeSafe(text.substr(0, indexRight), 14.f));
		float widthLeft = std::get<0>(font->getTextSizeSafe(text.substr(0, indexLeft), 14.f));

		if (std::abs(widthRight - x) < std::abs(widthLeft - x)) return indexRight;
		return indexLeft;
	}

	std::pair<int64_t, int64_t> TextInput::State::getWordRange(int64_t index) const {
		if (text.empty()) return {0, 0};
		auto idx = std::clamp(index, static_cast<int64_t>(0), static_cast<int64_t>(text.size()));
		if (idx == static_cast<int64_t>(text.size()) && idx > 0) idx--;

		auto isSpace = [&](int64_t i) {
			if (i < 0 || i >= static_cast<int64_t>(text.size())) return false;
			return text[i] == ' ';
		};
		bool targetIsSpace = isSpace(idx);

		int64_t s = idx;
		while (s > 0 && isSpace(s - 1) == targetIsSpace)
			s--;

		int64_t e = idx;
		while (e < static_cast<int64_t>(text.size()) && isSpace(e) == targetIsSpace)
			e++;

		return {s, e};
	}

	[[nodiscard]] float TextInput::State::getRelativeCursorX(const Gesture::State &state) const {
		float paddingLeft = widget->widget.padding.value_or(Padding{}).left;
		float marginLeft = widget->widget.margin.value_or(Margin{}).left;
		return state.getCursorPos().x - state.renderObject->getRect().left + scroll - paddingLeft - marginLeft;
	}

	void TextInput::State::handleMousePress(const Gesture::State &state) {
		if (!state.renderObject) return;

		float localX = getRelativeCursorX(state);

		int64_t newCursor = indexFromPos(localX);

		auto now = state.renderObject->app->frameStartTime;
		if (now - lastClickTime < std::chrono::milliseconds(500) && newCursor == lastClickedIndex)
			clickCount++;
		else
			clickCount = 1;
		lastClickTime = now;
		lastClickedIndex = newCursor;

		if (clickCount > 3) clickCount = 2;

		if (clickCount == 1) {
			dragType = DragType::Char;
			setState([&]() {
				cursor = newCursor;
				if (!state.inputState || !state.inputState->isKeyDown(GestureKey::leftShift))
					selectionStart = std::nullopt;
				pivot = newCursor;
			});
		} else if (clickCount == 2) {
			dragType = DragType::Word;
			auto range = getWordRange(newCursor);
			pivotRange = range;
			setState([&]() {
				selectionStart = range.first;
				cursor = range.second;
			});
		} else if (clickCount == 3) {
			dragType = DragType::Line;
			pivotRange = {0, static_cast<int64_t>(text.size())};
			setState([&]() {
				selectionStart = 0;
				cursor = static_cast<int64_t>(text.size());
			});
		}
	}

	void TextInput::State::handleMouseDrag(const Gesture::State &state) {
		if (!state.renderObject) return;

		float localX = getRelativeCursorX(state);

		int64_t newPos = indexFromPos(localX);

		if (dragType == DragType::Char) {
			setState([&]() {
				if (!selectionStart.has_value()) selectionStart = pivot;
				cursor = newPos;
			});
		} else if (dragType == DragType::Word) {
			auto currentWord = getWordRange(newPos);
			setState([&]() {
				if (newPos >= pivotRange.second) {
					selectionStart = pivotRange.first;
					cursor = currentWord.second;
				} else if (newPos <= pivotRange.first) {
					selectionStart = pivotRange.second;
					cursor = currentWord.first;
				} else {
					selectionStart = pivotRange.first;
					cursor = pivotRange.second;
				}
			});
		}
	}

	Child TextInput::State::getSelectionBox(uint32_t widthToStart) const {
		if (!selectionStart.has_value()) return nullptr;
		auto selectionMin = getSelectionMin();
		auto selectionMax = getSelectionMax();
		if (selectionMin != cursor) {
			std::tie(widthToStart, std::ignore) = font->getTextSizeSafe(
				text.substr(0, selectionMin),
				14.f
			);
		}
		auto theme = ThemeManager::getTheme();
		return Offset{
			.calculateContentBounds = [widthToStart](const Rect &bounds, const SingleChildRenderObject &) -> Rect {
				auto ret = bounds;
				ret.offset({static_cast<float>(widthToStart), 0.f});
				return ret;
			},
			.child = Box{
				.widget{
					.width = Size::Wrap,
					.height = Size::Wrap,
				},
				.color = theme.accent,
				.child = Text{
					.text = text.substr(selectionMin, selectionMax - selectionMin),
					.color = theme.accent.isLight() ? Color::black : Color::white,
				},
			},
		};
	}

	[[nodiscard]] Child TextInput::State::getCursorBox(uint32_t widthToCursor) const {
		if (!widget->active) return nullptr;
		return Offset{
			.calculateContentBounds = [widthToCursor](const Rect &bounds, const SingleChildRenderObject &) -> Rect {
				auto ret = bounds;
				ret.offset({static_cast<float>(widthToCursor), 0.f});
				return ret;
			},
			.child = Box{
				.widget{
					.width = 1.f,
				},
			},
		};
	}

	Child TextInput::State::build(const Element &) {
		auto [widthToCursor, _] = font->getTextSizeSafe(
			text.substr(0, cursor),
			14.f
		);

		this->element->addPostLayoutTask([this]() {
			if (this->cachedScrollData != *this->scrollController) {
				this->cachedScrollData = *this->scrollController;
				this->element->markNeedsRebuild();
			}
		});

		auto args = widget->widget;
		args.height = args.height.value_or(Size::Shrink);
		args.padding = args.padding.value_or(Padding{}.withRight(1.f));

		auto minScroll = std::max(static_cast<float>(widthToCursor) - cachedScrollData.viewMainAxis, 0.f);
		auto maxScroll = std::min(static_cast<float>(widthToCursor), std::max(cachedScrollData.contentMainAxis - cachedScrollData.viewMainAxis, 0.f));
		// Min scroll can be bigger than max scroll for a single build
		minScroll = std::min(minScroll, maxScroll);
		scroll = std::clamp(scroll, minScroll, maxScroll);

		return Gesture{
			.onPress = [this](const Gesture::State &state) {
				handleMousePress(state);
			},
			.onDrag = [this](const Gesture::State &state) {
				handleMouseDrag(state);
			},
			.onUpdate = [this](const Gesture::State &state) {
				if (!widget->active)
					return;

				auto lastCursor = cursor;

				// Make sure the cursors are not out of bounds
				clampCursors();

				handleTextInput(state);
				handleBackspace(state);
				handleDelete(state);

				handleLeftArrow(state);
				handleRightArrow(state);

				handleHome(state);
				handleEnd(state);

				handleEscape(state);

				handleSelectAll(state);
				handleCopy(state);
				handleCut(state);
				handlePaste(state);

				if (lastCursor != cursor)
					this->element->markNeedsReposition();
			},
			.child = Scrollable{
				.widget = args,
				.direction = Axis::Horizontal,
				.scroll = scroll,
				.controller = scrollController,
				.children{Stack{
					.widget{
						.width = Size::Wrap,
						.height = Size::Wrap,
					},
					.children{
						Text{
							.text = text,
						},
						getSelectionBox(widthToCursor),
						getCursorBox(widthToCursor),
					},
				}},
			},
		};
	}
}// namespace squi