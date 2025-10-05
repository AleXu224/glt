#include "widgets/textInput.hpp"

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
		setState([this, &newText]() {
			text = newText;
		});
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

	uint64_t TextInput::State::getPrevWordStart() const {
		auto pos = text.find_last_of(' ', cursor - 1);
		if (pos == std::string::npos) pos = 0;
		return pos;
	}

	uint64_t TextInput::State::getNextWordStart() const {
		auto pos = text.find_first_of(' ', cursor + 1);
		if (pos == std::string::npos) pos = text.size();
		return pos;
	}

	void TextInput::State::handleBackspace(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) {
			if (selectionStart.has_value()) {
				clearSelection();
				return;
			}
			const auto &keyState = key.value();
			if (keyState.mods & GLFW_MOD_CONTROL && cursor > 0) {
				auto pos = getPrevWordStart();

				setText(std::format("{}{}", text.substr(0, pos), text.substr(cursor)));
				setState([&]() {
					cursor = static_cast<int64_t>(pos);
				});
			} else if (cursor > 0) {
				setText(std::format("{}{}", text.substr(0, cursor - 1), text.substr(cursor)));
				setState([&]() {
					--cursor;
				});
			}
		}
	}

	void TextInput::State::handleDelete(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_DELETE)) {
			if (selectionStart.has_value()) {
				clearSelection();
			} else {
				const auto &keyState = key.value();
				if (keyState.mods & GLFW_MOD_CONTROL && cursor < static_cast<int64_t>(text.size())) {
					auto pos = getNextWordStart();

					setText(std::format("{}{}", text.substr(0, cursor), text.substr(pos)));
				} else if (cursor < static_cast<int64_t>(text.size())) {
					setText(std::format("{}{}", text.substr(0, cursor), text.substr(cursor + 1)));
				}
			}
		}
	}

	void TextInput::State::handleLeftArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_LEFT)) {
			bool removedSelection = false;
			if (key->mods & GLFW_MOD_SHIFT && cursor > 0) {
				if (!selectionStart.has_value()) setState([&]() {
					selectionStart = cursor;
				});
			} else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value()) {
				clampCursors();
				setState([&]() {
					cursor = getSelectionMin();
					selectionStart = std::nullopt;
				});
				removedSelection = true;
			}

			if (cursor > 0 && !removedSelection) {
				if (key->mods & GLFW_MOD_CONTROL) {
					auto pos = getPrevWordStart();

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
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_RIGHT)) {
			bool removedSelection = false;
			if (key->mods & GLFW_MOD_SHIFT && cursor < static_cast<int64_t>(text.size())) {
				if (!selectionStart.has_value()) setState([&]() {
					selectionStart = cursor;
				});
			} else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value()) {
				clampCursors();
				setState([&]() {
					cursor = getSelectionMax();
					selectionStart = std::nullopt;
				});
				removedSelection = true;
			}

			if (cursor < static_cast<int64_t>(text.size()) && !removedSelection) {
				if (key->mods & GLFW_MOD_CONTROL) {
					auto pos = getNextWordStart();

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
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_HOME)) {
			if (key->mods & GLFW_MOD_SHIFT && cursor > 0 && !selectionStart.has_value())
				setState([&]() {
					selectionStart = cursor;
				});
			else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value())
				setState([&]() {
					selectionStart = std::nullopt;
				});

			setState([&]() {
				cursor = std::min<int64_t>(cursor, 0);
			});
		}
	}

	void TextInput::State::handleEnd(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_END)) {
			if (key->mods & GLFW_MOD_SHIFT && cursor < static_cast<int64_t>(text.size()) && !selectionStart.has_value())
				setState([&]() {
					selectionStart = cursor;
				});
			else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value())
				setState([&]() {
					selectionStart = std::nullopt;
				});

			setState([&]() {
				cursor = std::max(cursor, static_cast<int64_t>(text.size()));
			});
		}
	}

	void TextInput::State::handleEscape(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
			selectionStart = std::nullopt;
		}
	}

	void TextInput::State::handleSelectAll(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_A); key && key->mods & GLFW_MOD_CONTROL) {
			setState([&]() {
				selectionStart = 0;
				cursor = static_cast<int64_t>(text.size());
			});
		}
	}

	void TextInput::State::handleCopy(const Gesture::State &state) const {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_C); key && key->mods & GLFW_MOD_CONTROL && selectionStart.has_value()) {
			auto textToCopy = text.substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
			glfwSetClipboardString(nullptr, textToCopy.c_str());
		}
	}

	void TextInput::State::handleCut(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_X); key && key->mods & GLFW_MOD_CONTROL && selectionStart.has_value()) {
			auto textToCopy = text.substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
			glfwSetClipboardString(nullptr, textToCopy.c_str());
			clearSelection();
		}
	}

	void TextInput::State::handlePaste(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_V); key && key->mods & GLFW_MOD_CONTROL) {
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