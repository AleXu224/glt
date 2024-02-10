#include "textInput.hpp"
#include "box.hpp"
#include "cstdint"
#include "gestureDetector.hpp"
#include "text.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include "window.hpp"

using namespace squi;

TextInput::Impl::Impl(const TextInput &args)
	: Widget(args.widget, Widget::Flags::Default()),
	  text(args.text),
	  gd(GestureDetector{}.mount(*this)) {
	addChild(Text{
		.text{""},
		.fontSize = args.fontSize,
		.font{args.font},
		.color{args.color},
	});
	// Selection
	addChild(Box{
		.color{0x0078D4FF},
	});
	// Selected text
	addChild(Text{
		.text{""},
		.fontSize = args.fontSize,
		.font{args.font},
		.color{0xFFFFFFFF},
	});
	// Cursor
	addChild(Box{
		.widget{
			.width = 2.f,
		},
		.color{0xFFFFFFFF},
	});
}

void TextInput::Impl::clampCursors() {
	const auto text = getText();
	cursor = std::clamp(cursor, 0ll, static_cast<int64_t>(text.size()));
	if (selectionStart.has_value()) {
		selectionStart = std::clamp(selectionStart.value_or(0), 0ll, static_cast<int64_t>(text.size()));
		if (cursor == selectionStart.value()) selectionStart = std::nullopt;
	}
}

int64_t TextInput::Impl::getSelectionMin() {
	clampCursors();
	if (!selectionStart.has_value()) return cursor;
	return std::min(selectionStart.value(), cursor);
}

int64_t TextInput::Impl::getSelectionMax() {
	clampCursors();
	if (!selectionStart.has_value()) return cursor;
	return std::max(selectionStart.value(), cursor);
}

std::string_view TextInput::Impl::getText() {
	if (text.has_value()) return text.value().get();
	else {
		auto &children = getChildren();
		auto &text = reinterpret_cast<Text::Impl &>(*children[0]);
		return text.getText();
	}
}

void TextInput::Impl::setText(std::string_view text) {
	if (this->text.has_value()) this->text.value().get() = text;
	auto &children = getChildren();
	auto &textWidget = reinterpret_cast<Text::Impl &>(*children[0]);
	textWidget.setText(text);
}

void TextInput::Impl::clearSelection() {
	clampCursors();
	if (!selectionStart.has_value()) return;
	const auto min = getSelectionMin();
	const auto max = getSelectionMax();

	auto text = getText();
	std::string newText = std::format("{}{}", std::string(text.substr(0, min)), std::string(text.substr(max)));
	setText(newText);
	cursor = min;
	selectionStart = std::nullopt;
}

void TextInput::Impl::onUpdate() {
	auto &cursorWidget = reinterpret_cast<Box::Impl &>(*getChildren()[3]);
	if (!gd.active) {
		cursorWidget.setVisible(false);
		return;
	}
	cursorWidget.setVisible(true);

	auto &children = getChildren();
	auto &text = reinterpret_cast<Text::Impl &>(*children[0]);
	// auto &selectedText = reinterpret_cast<Text::Impl &>(*children[2]);
	const auto oldCursor = cursor;
	const auto oldSelectionStart = selectionStart;

	clampCursors();

	if (!GestureDetector::g_textInput.empty()) {
		clearSelection();
		const auto textValue = text.getText();
		setText(std::format("{}{}{}", textValue.substr(0, cursor), GestureDetector::g_textInput, textValue.substr(cursor)));
		cursor += static_cast<int64_t>(GestureDetector::g_textInput.size());
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) {
		if (selectionStart.has_value()) {
			clearSelection();
		} else {
			const auto textValue = text.getText();
			const auto &keyState = key.value();
			if (keyState.mods & GLFW_MOD_CONTROL && cursor > 0) {
				auto pos = textValue.find_last_of(' ', cursor - 1);
				if (pos == std::string::npos) pos = 0;

				setText(std::format("{}{}", textValue.substr(0, pos), textValue.substr(cursor)));
				cursor = static_cast<int64_t>(pos);
			} else if (cursor > 0) {
				setText(std::format("{}{}", textValue.substr(0, cursor - 1), textValue.substr(cursor)));
				--cursor;
			}
		}
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_DELETE)) {
		if (selectionStart.has_value()) {
			clearSelection();
		} else {
			const auto textValue = text.getText();
			const auto &keyState = key.value();
			if (keyState.mods & GLFW_MOD_CONTROL && cursor < static_cast<int64_t>(textValue.size())) {
				auto pos = textValue.find_first_of(' ', cursor + 1);
				if (pos == std::string::npos) pos = textValue.size();

				setText(std::format("{}{}", textValue.substr(0, cursor), textValue.substr(pos)));
			} else if (cursor < static_cast<int64_t>(textValue.size())) {
				setText(std::format("{}{}", textValue.substr(0, cursor), textValue.substr(cursor + 1)));
			}
		}
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_LEFT)) {
		bool removedSelection = false;
		if (key->mods & GLFW_MOD_SHIFT && cursor > 0) {
			if (!selectionStart.has_value()) selectionStart = cursor;
		} else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value()) {
			cursor = getSelectionMin();
			selectionStart = std::nullopt;
			removedSelection = true;
		}

		if (cursor > 0 && !removedSelection) {
			if (key->mods & GLFW_MOD_CONTROL) {
				const auto textValue = text.getText();
				auto pos = textValue.find_last_of(' ', cursor - 1);
				if (pos == std::string::npos) pos = 0;

				cursor = static_cast<int64_t>(pos);
			} else {
				--cursor;
			}
		}
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_RIGHT)) {
		const auto textValue = text.getText();
		bool removedSelection = false;
		if (key->mods & GLFW_MOD_SHIFT && cursor < static_cast<int64_t>(textValue.size())) {
			if (!selectionStart.has_value()) selectionStart = cursor;
		} else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value()) {
			cursor = getSelectionMax();
			selectionStart = std::nullopt;
			removedSelection = true;
		}

		if (cursor < static_cast<int64_t>(textValue.size()) && !removedSelection) {
			if (key->mods & GLFW_MOD_CONTROL) {
				auto pos = textValue.find_first_of(' ', cursor + 1);
				if (pos == std::string::npos) pos = textValue.size();

				cursor = static_cast<int64_t>(pos);
			} else {
				++cursor;
			}
		}
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_HOME)) {
		if (key->mods & GLFW_MOD_SHIFT && cursor > 0 && !selectionStart.has_value())
			selectionStart = cursor;
		else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value())
			selectionStart = std::nullopt;

		if (cursor > 0) cursor = 0;
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_END)) {
		const auto textValue = text.getText();
		if (key->mods & GLFW_MOD_SHIFT && cursor < static_cast<int64_t>(textValue.size()) && !selectionStart.has_value())
			selectionStart = cursor;
		else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value())
			selectionStart = std::nullopt;

		if (cursor < static_cast<int64_t>(textValue.size()))
			cursor = static_cast<int64_t>(textValue.size());
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
		selectionStart = std::nullopt;
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_A)) {
		if (key->mods & GLFW_MOD_CONTROL) {
			selectionStart = 0;
			const auto textValue = text.getText();
			cursor = static_cast<int64_t>(textValue.size());
		}
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_C)) {
		if (key->mods & GLFW_MOD_CONTROL && selectionStart.has_value()) {
			const std::string textToCopy = std::string(getText().substr(getSelectionMin(), getSelectionMax() - getSelectionMin()));
			glfwSetClipboardString(nullptr, textToCopy.c_str());
		}
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_X)) {
		if (key->mods & GLFW_MOD_CONTROL && selectionStart.has_value()) {
			const std::string textToCopy = std::string(getText().substr(getSelectionMin(), getSelectionMax() - getSelectionMin()));
			glfwSetClipboardString(nullptr, textToCopy.c_str());
			clearSelection();
		}
	} else if (const auto key = GestureDetector::getKeyPressedOrRepeat(GLFW_KEY_V)) {
		if (key->mods & GLFW_MOD_CONTROL) {
			const auto clipboardText = glfwGetClipboardString(nullptr);
			if (clipboardText) {
				clearSelection();
				const auto clipboardString = std::string_view(clipboardText);
				const auto textValue = text.getText();
				setText(std::format("{}{}{}", textValue.substr(0, cursor), clipboardString, textValue.substr(cursor)));
				cursor += static_cast<int64_t>(clipboardString.size());
			}
		}
	}

	if (oldCursor != cursor) {
		reLayout();
	}
	if (oldCursor != cursor || oldSelectionStart != selectionStart) {
		auto &selectionWidget = reinterpret_cast<Box::Impl &>(*children[1]);
		auto &selectionTextWidget = reinterpret_cast<Text::Impl &>(*children[2]);

		if (selectionStart.has_value()) {
			selectionWidget.setVisible(true);
			selectionTextWidget.setVisible(true);
			auto textVal = getText();
			const auto newText = textVal.substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
			const auto [width, height] = text.getTextSize(newText);
			selectionWidget.setWidth(static_cast<float>(width));
			selectionTextWidget.setText(newText);
		} else {
			selectionWidget.setVisible(false);
			selectionTextWidget.setVisible(false);
			selectionTextWidget.setText("");
		}
	}
}

vec2 TextInput::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) {
	auto &children = getChildren();
	for (auto &child: children) {
		const auto size = child->layout(maxSize.withX(std::numeric_limits<float>::max()), {});
		minSize.x = std::clamp(size.x, minSize.x, maxSize.x);
		minSize.y = std::clamp(size.y, minSize.y, maxSize.y);
	}

	return minSize;
}

void TextInput::Impl::arrangeChildren(vec2 &pos) {
	auto &children = getChildren();
	auto &textWidget = reinterpret_cast<Text::Impl &>(*children[0]);
	auto &selectionWidget = reinterpret_cast<Box::Impl &>(*children[1]);
	auto &selectedTextWidget = reinterpret_cast<Text::Impl &>(*children[2]);
	auto &cursorWidget = reinterpret_cast<Box::Impl &>(*children[3]);

	const auto text = getText();
	const auto [startToCursorWidth, startToCursorHeight] = textWidget.getTextSize(text.substr(0, cursor));
	const auto contentWidth = std::max(getContentRect().width(), 0.f);
	scroll = std::clamp(scroll, static_cast<float>(startToCursorWidth) - contentWidth, static_cast<float>(startToCursorWidth));

	const auto contentPos = pos + state.margin.getPositionOffset() + state.padding.getPositionOffset() - vec2{scroll, 0};

	textWidget.arrange(contentPos);
	if (selectionStart.has_value()) {
		const auto [width, height] = textWidget.getTextSize(text.substr(0, getSelectionMin()));
		selectionWidget.arrange(contentPos.withXOffset(static_cast<float>(width)));
		selectedTextWidget.arrange(contentPos.withXOffset(static_cast<float>(width)));
	}
	cursorWidget.arrange(contentPos.withXOffset(static_cast<float>(startToCursorWidth)));
}

void TextInput::Impl::drawChildren() {
	auto &children = getChildren();
	auto &textWidget = reinterpret_cast<Text::Impl &>(*children[0]);
	auto &selectionWidget = reinterpret_cast<Box::Impl &>(*children[1]);
	auto &selectedTextWidget = reinterpret_cast<Text::Impl &>(*children[2]);
	auto &cursorWidget = reinterpret_cast<Box::Impl &>(*children[3]);

	auto &instance = Window::of(this).engine.instance;
	instance.pushScissor(getRect());

	textWidget.draw();
	selectionWidget.draw();
	selectedTextWidget.draw();
	cursorWidget.draw();

	instance.popScissor();
}
TextInput::operator Child() const {
	return GestureDetector{.child{std::make_unique<Impl>(*this)}};
}

void squi::TextInput::Impl::setActive(bool active) {
	gd.active = active;
}
