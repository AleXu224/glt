#include "widgets/textInput.hpp"

#include "theme.hpp"
#include "widgets/box.hpp"
#include "widgets/scrollable.hpp"
#include "widgets/stack.hpp"
#include "widgets/text.hpp"

namespace squi {
	void TextInput::State::clampCursors() {
		auto newCursor = std::clamp(controller->cursor, static_cast<int64_t>(0), static_cast<int64_t>(controller->text.size()));
		if (newCursor != controller->cursor) {
			setState([this, newCursor]() {
				controller->cursor = newCursor;
			});
		}
		if (controller->selectionStart.has_value()) {
			std::optional<int64_t> newSelectionStart = std::clamp(controller->selectionStart.value_or(0), static_cast<int64_t>(0), static_cast<int64_t>(controller->text.size()));
			if (controller->cursor == controller->selectionStart.value())
				newSelectionStart = std::nullopt;

			if (newSelectionStart != controller->selectionStart) {
				setState([this, newSelectionStart]() {
					controller->selectionStart = newSelectionStart;
				});
			}
		}
	}

	int64_t TextInput::State::getSelectionMin() const {
		if (!controller->selectionStart.has_value()) return controller->cursor;
		return std::min(controller->selectionStart.value(), controller->cursor);
	}

	int64_t TextInput::State::getSelectionMax() const {
		if (!controller->selectionStart.has_value()) return controller->cursor;
		return std::max(controller->selectionStart.value(), controller->cursor);
	}

	void TextInput::State::setText(const std::string &text) {
		setState([this, &text]() {
			controller->text = text;
		});
	}

	void TextInput::State::clearSelection() {
		clampCursors();
		if (!controller->selectionStart.has_value()) return;
		const auto min = getSelectionMin();
		const auto max = getSelectionMax();

		std::string newText = std::format("{}{}", std::string(controller->text.substr(0, min)), std::string(controller->text.substr(max)));
		setText(newText);
		setState([&]() {
			controller->cursor = min;
			controller->selectionStart = std::nullopt;
		});
	}

	void TextInput::State::handleTextInput(const Gesture::State &state) {
		if (!state.inputState->g_textInput.empty()) {
			clearSelection();
			setState([this, &state]() {
				controller->text = std::format("{}{}{}", controller->text.substr(0, controller->cursor), state.inputState->g_textInput, controller->text.substr(controller->cursor));
				controller->cursor += static_cast<int64_t>(state.inputState->g_textInput.size());
			});
		}
	}

	uint64_t TextInput::State::getPrevWordStart() const {
		auto pos = controller->text.find_last_of(' ', controller->cursor - 1);
		if (pos == std::string::npos) pos = 0;
		return pos;
	}

	uint64_t TextInput::State::getNextWordStart() const {
		auto pos = controller->text.find_first_of(' ', controller->cursor + 1);
		if (pos == std::string::npos) pos = controller->text.size();
		return pos;
	}

	void TextInput::State::handleBackspace(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) {
			if (controller->selectionStart.has_value()) {
				clearSelection();
				return;
			}
			const auto &keyState = key.value();
			if (keyState.mods & GLFW_MOD_CONTROL && controller->cursor > 0) {
				auto pos = getPrevWordStart();

				setText(std::format("{}{}", controller->text.substr(0, pos), controller->text.substr(controller->cursor)));
				setState([&]() {
					controller->cursor = static_cast<int64_t>(pos);
				});
			} else if (controller->cursor > 0) {
				setText(std::format("{}{}", controller->text.substr(0, controller->cursor - 1), controller->text.substr(controller->cursor)));
				setState([&]() {
					--controller->cursor;
				});
			}
		}
	}

	void TextInput::State::handleDelete(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_DELETE)) {
			if (controller->selectionStart.has_value()) {
				clearSelection();
			} else {
				const auto &keyState = key.value();
				if (keyState.mods & GLFW_MOD_CONTROL && controller->cursor < static_cast<int64_t>(controller->text.size())) {
					auto pos = getNextWordStart();

					setText(std::format("{}{}", controller->text.substr(0, controller->cursor), controller->text.substr(pos)));
				} else if (controller->cursor < static_cast<int64_t>(controller->text.size())) {
					setText(std::format("{}{}", controller->text.substr(0, controller->cursor), controller->text.substr(controller->cursor + 1)));
				}
			}
		}
	}

	void TextInput::State::handleLeftArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_LEFT)) {
			bool removedSelection = false;
			if (key->mods & GLFW_MOD_SHIFT && controller->cursor > 0) {
				if (!controller->selectionStart.has_value()) setState([&]() {
					controller->selectionStart = controller->cursor;
				});
			} else if (!(key->mods & GLFW_MOD_SHIFT) && controller->selectionStart.has_value()) {
				clampCursors();
				setState([&]() {
					controller->cursor = getSelectionMin();
					controller->selectionStart = std::nullopt;
				});
				removedSelection = true;
			}

			if (controller->cursor > 0 && !removedSelection) {
				if (key->mods & GLFW_MOD_CONTROL) {
					auto pos = getPrevWordStart();

					setState([&]() {
						controller->cursor = static_cast<int64_t>(pos);
					});
				} else {
					setState([&]() {
						--controller->cursor;
					});
				}
			}
		}
	}

	void TextInput::State::handleRightArrow(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_RIGHT)) {
			bool removedSelection = false;
			if (key->mods & GLFW_MOD_SHIFT && controller->cursor < static_cast<int64_t>(controller->text.size())) {
				if (!controller->selectionStart.has_value()) setState([&]() {
					controller->selectionStart = controller->cursor;
				});
			} else if (!(key->mods & GLFW_MOD_SHIFT) && controller->selectionStart.has_value()) {
				clampCursors();
				setState([&]() {
					controller->cursor = getSelectionMax();
					controller->selectionStart = std::nullopt;
				});
				removedSelection = true;
			}

			if (controller->cursor < static_cast<int64_t>(controller->text.size()) && !removedSelection) {
				if (key->mods & GLFW_MOD_CONTROL) {
					auto pos = getNextWordStart();

					setState([&]() {
						controller->cursor = static_cast<int64_t>(pos);
					});
				} else {
					setState([&]() {
						++controller->cursor;
					});
				}
			}
		}
	}

	void TextInput::State::handleHome(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_HOME)) {
			if (key->mods & GLFW_MOD_SHIFT && controller->cursor > 0 && !controller->selectionStart.has_value())
				setState([&]() {
					controller->selectionStart = controller->cursor;
				});
			else if (!(key->mods & GLFW_MOD_SHIFT) && controller->selectionStart.has_value())
				setState([&]() {
					controller->selectionStart = std::nullopt;
				});

			setState([&]() {
				controller->cursor = std::min<int64_t>(controller->cursor, 0);
			});
		}
	}

	void TextInput::State::handleEnd(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_END)) {
			if (key->mods & GLFW_MOD_SHIFT && controller->cursor < static_cast<int64_t>(controller->text.size()) && !controller->selectionStart.has_value())
				setState([&]() {
					controller->selectionStart = controller->cursor;
				});
			else if (!(key->mods & GLFW_MOD_SHIFT) && controller->selectionStart.has_value())
				setState([&]() {
					controller->selectionStart = std::nullopt;
				});

			setState([&]() {
				controller->cursor = std::max(controller->cursor, static_cast<int64_t>(controller->text.size()));
			});
		}
	}

	void TextInput::State::handleEscape(const Gesture::State &state) const {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
			controller->selectionStart = std::nullopt;
		}
	}

	void TextInput::State::handleSelectAll(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_A); key && key->mods & GLFW_MOD_CONTROL) {
			setState([&]() {
				controller->selectionStart = 0;
				controller->cursor = static_cast<int64_t>(controller->text.size());
			});
		}
	}

	void TextInput::State::handleCopy(const Gesture::State &state) const {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_C); key && key->mods & GLFW_MOD_CONTROL && controller->selectionStart.has_value()) {
			auto textToCopy = controller->text.substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
			glfwSetClipboardString(nullptr, textToCopy.c_str());
		}
	}

	void TextInput::State::handleCut(const Gesture::State &state) {
		if (const auto key = state.inputState->getKeyPressedOrRepeat(GLFW_KEY_X); key && key->mods & GLFW_MOD_CONTROL && controller->selectionStart.has_value()) {
			auto textToCopy = controller->text.substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
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
			const auto &textValue = controller->text;
			setText(std::format("{}{}{}", textValue.substr(0, controller->cursor), clipboardString, textValue.substr(controller->cursor)));
			setState([&]() {
				controller->cursor += static_cast<int64_t>(clipboardString.size());
			});
		}
	}

	Child TextInput::State::getSelectionBox(uint32_t widthToStart) const {
		if (!controller->selectionStart.has_value()) return nullptr;
		auto selectionMin = getSelectionMin();
		auto selectionMax = getSelectionMax();
		if (selectionMin != controller->cursor) {
			std::tie(widthToStart, std::ignore) = font->getTextSizeSafe(
				controller->text.substr(0, selectionMin),
				14.f
			);
		}
		auto theme = ThemeManager::getTheme();
		return Box{
			.widget{
				.width = Size::Wrap,
				.height = Size::Wrap,
				.margin = Margin{}.withLeft(static_cast<float>(widthToStart)),
			},
			.color = theme.accent,
			.child = Text{
				.text = controller->text.substr(
					selectionMin,
					selectionMax - selectionMin
				),
				.color = theme.accent.isLight() ? Color::black : Color::white,
			},
		};
	}

	Child TextInput::State::build(const Element &) {
		auto [widthToCursor, _] = font->getTextSizeSafe(
			controller->text.substr(0, controller->cursor),
			14.f
		);

		return Gesture{
			.onUpdate = [this](const Gesture::State &state) {
				if (!widget->active)
					return;

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
			},
			.child = Scrollable{
				.widget{
					.height = Size::Shrink,
				},
				.direction = Axis::Horizontal,
				.children{Stack{
					.widget{
						.width = Size::Wrap,
						.height = Size::Wrap,
					},
					.children{
						Text{
							.text = controller->text,
						},
						getSelectionBox(widthToCursor),
						Box{
							.widget{
								.width = 1.f,
								.margin = Margin{}.withLeft(static_cast<float>(widthToCursor)),
							},
						},
					},
				}},
			},
		};
	}
}// namespace squi