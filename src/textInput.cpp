#include "gestureDetector.hpp"
#define NOMINMAX
#include "box.hpp"
#include "cstdint"
#include "fontStore.hpp"
#include "renderer.hpp"
#include "text.hpp"
#include "textInput.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <limits>
#include <optional>
#include <stdint.h>



using namespace squi;

TextInput::Impl::Impl(const TextInput &args)
	: Widget(args.widget, Widget::Options{
							  .shouldDrawChildren = false,
							  .shouldHandleLayout = false,
							  .shouldArrangeChildren = false,
						  }) {
	addChild(Text{
		.text{""},
		.fontSize = args.fontSize,
		.fontPath{args.font},
		.color{args.color},
	});
	// Selection
	addChild(Box{
		.color{Color::HEX(0x0078D4FF)},
	});
	// Selected text
	addChild(Text{
		.text{""},
		.fontSize = args.fontSize,
		.fontPath{args.font},
		.color{Color::HEX(0xFFFFFFFF)},
	});
	// Cursor
	addChild(Box{
		.widget{
			.width = 2.f,
		},
		.color{Color::HEX(0xFFFFFFFF)},
	});
}

// TODO: Fix the horrible performance of this
void TextInput::Impl::onUpdate() {
	if (!gd) return;
	auto &gd = *this->gd;
	if (!gd.active) return;

	auto &children = getChildren();
	auto &text = reinterpret_cast<Text::Impl &>(*children[0]);
	auto &selectedText = reinterpret_cast<Text::Impl &>(*children[2]);

	const auto oldValue = text.getText();
	auto newValue = std::string(oldValue);

	const auto oldCursor = cursor;
	const auto oldSelectionStart = (std::min)(cursor, selectionStart.value_or(-1));
	const auto oldSelectionEnd = (std::max)(cursor, selectionStart.value_or(-1));

	cursor = (std::min)(cursor, static_cast<uint32_t>(newValue.size()));
	if (selectionStart.has_value()) selectionStart = (std::min)(selectionStart.value_or(0), static_cast<uint32_t>(newValue.size()));

	const auto removeSelectedRegion = [&]() {
		if (!selectionStart.has_value()) return;
		uint32_t selectStartPos = (std::min)(cursor, selectionStart.value());
		uint32_t selectEndPos = (std::max)(cursor, selectionStart.value());
		newValue.replace(selectStartPos, selectEndPos - selectStartPos, "");
		selectionStart = std::nullopt;
		cursor = selectStartPos > static_cast<uint32_t>(newValue.size()) ? static_cast<int>(newValue.size()) : selectStartPos;
	};
	if (gd.charInput) {
		if (selectionStart.has_value()) removeSelectedRegion();
		newValue.insert(cursor, std::string{static_cast<char>(gd.charInput)});
		++cursor;
	}
	// TODO: add more keybinds
	// https://www.howtogeek.com/115664/42-text-editing-keyboard-shortcuts-that-work-almost-everywhere/
	// At least usable for now
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_BACKSPACE) && !newValue.empty()) {
		if (selectionStart.has_value()) {
			removeSelectedRegion();
		} else if (cursor != 0) {
			newValue.replace(cursor - 1, 1, "");
			--cursor;
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_BACKSPACE, GLFW_MOD_CONTROL) && !newValue.empty()) {
		if (selectionStart.has_value()) {
			removeSelectedRegion();
		} else {
			uint32_t step = cursor;
			int64_t i = cursor - 1;
			for (; i >= 0; i--) {
				if (!isspace(newValue.at(i))) break;
				step = i;
			}
			for (; i >= 0; i--) {
				if (isspace(newValue.at(i))) break;
				step = i;
			}
			if (step == cursor && step > 0) --step;
			newValue.replace(step, cursor - step, "");
			cursor = step;
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_DELETE) && !newValue.empty()) {
		if (selectionStart.has_value()) {
			removeSelectedRegion();
		} else if (cursor != newValue.size()) {
			newValue.replace(cursor, 1, "");
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_DELETE, GLFW_MOD_CONTROL) && !newValue.empty()) {
		if (selectionStart.has_value()) {
			removeSelectedRegion();
		} else {
			uint32_t step = cursor;
			uint32_t i = cursor + 1;
			for (; i <= newValue.size(); i++) {
				if (!isspace(newValue.at(i - 1))) break;
				step = i;
			}
			for (; i <= newValue.size(); i++) {
				if (isspace(newValue.at(i - 1))) break;
				step = i;
			}
			if (step == cursor && step < newValue.size()) ++step;
			newValue.replace(cursor, step - cursor, "");
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_LEFT)) {
		if (selectionStart.has_value()) cursor = (std::min)(cursor, selectionStart.value());
		else
			cursor = cursor == 0 ? 0 : --cursor;
		selectionStart = std::nullopt;
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_LEFT, GLFW_MOD_SHIFT)) {
		if (!selectionStart.has_value()) selectionStart = cursor;
		cursor = cursor == 0 ? 0 : --cursor;
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_LEFT, GLFW_MOD_CONTROL)) {
		selectionStart = std::nullopt;
		if (cursor != 0) {
			uint32_t step = cursor;
			int64_t i = cursor - 1;
			// Step through all the whitespace characters until it hits a non whitespace
			// then it should go through all the non whitespace characters
			// This is the behaviour most people expect out of this
			for (; i >= 0; i--) {
				if (!isspace(newValue.at(i))) break;
				step = i;
			}
			for (; i >= 0; i--) {
				if (isspace(newValue.at(i))) break;
				step = i;
			}
			if (step == cursor && step > 0) --cursor;
			else
				cursor = step;
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_LEFT, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) {
		if (!selectionStart.has_value()) selectionStart = cursor;
		if (cursor != 0) {
			uint32_t step = cursor;
			int64_t i = cursor - 1;
			for (; i >= 0; i--) {
				if (!isspace(newValue.at(i))) break;
				step = i;
			}
			for (; i >= 0; i--) {
				if (isspace(newValue.at(i))) break;
				step = i;
			}
			if (step == cursor && step > 0) --cursor;
			else
				cursor = step;
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_RIGHT)) {
		if (selectionStart.has_value())
			cursor = (std::max)(cursor, selectionStart.value());
		else
			cursor = ++cursor > newValue.size() ? newValue.size() : cursor;
		selectionStart = std::nullopt;
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_RIGHT, GLFW_MOD_SHIFT)) {
		if (!selectionStart.has_value()) selectionStart = cursor;
		cursor = ++cursor > newValue.size() ? newValue.size() : cursor;
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_RIGHT, GLFW_MOD_CONTROL)) {
		selectionStart = std::nullopt;
		if (cursor != newValue.size()) {
			uint32_t step = cursor;
			uint32_t i = cursor + 1;
			for (; i <= newValue.size(); i++) {
				if (!isspace(newValue.at(i - 1))) break;
				step = i;
			}
			for (; i <= newValue.size(); i++) {
				if (isspace(newValue.at(i - 1))) break;
				step = i;
			}
			if (step == cursor && step < newValue.size()) ++cursor;
			else
				cursor = step;
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_RIGHT, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) {
		if (!selectionStart.has_value()) selectionStart = cursor;
		if (cursor != newValue.size()) {
			uint32_t step = cursor;
			uint32_t i = cursor + 1;
			for (; i <= newValue.size(); i++) {
				if (!isspace(newValue.at(i - 1))) break;
				step = i;
			}
			for (; i <= newValue.size(); i++) {
				if (isspace(newValue.at(i - 1))) break;
				step = i;
			}
			if (step == cursor && step < newValue.size()) ++cursor;
			else
				cursor = step;
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_END)) {
		selectionStart = std::nullopt;
		cursor = static_cast<int>(newValue.size());
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_END, GLFW_MOD_SHIFT)) {
		if (!selectionStart.has_value()) selectionStart = cursor;
		cursor = static_cast<int>(newValue.size());
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_HOME)) {
		selectionStart = std::nullopt;
		cursor = 0;
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_HOME, GLFW_MOD_SHIFT)) {
		if (!selectionStart.has_value()) selectionStart = cursor;
		cursor = 0;
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
		selectionStart = std::nullopt;
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_V, GLFW_MOD_CONTROL)) {
		const char *clipboardContents = glfwGetClipboardString(nullptr);
		if (clipboardContents) {
			const std::string output{clipboardContents};
			if (selectionStart.has_value()) removeSelectedRegion();
			newValue.insert(cursor, output);
			cursor += static_cast<int>(output.size());
		}
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_C, GLFW_MOD_CONTROL) && selectionStart.has_value()) {
		uint32_t selectStartPos = (std::min)(cursor, selectionStart.value());
		uint32_t selectEndPos = (std::max)(cursor, selectionStart.value());

		glfwSetClipboardString(nullptr, newValue.substr(selectStartPos, selectEndPos - selectStartPos).c_str());
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_X, GLFW_MOD_CONTROL) && selectionStart.has_value()) {
		uint32_t selectStartPos = (std::min)(cursor, selectionStart.value());
		uint32_t selectEndPos = (std::max)(cursor, selectionStart.value());
		glfwSetClipboardString(nullptr, newValue.substr(selectStartPos, selectEndPos - selectStartPos).c_str());
		removeSelectedRegion();
	}
	if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_A, GLFW_MOD_CONTROL)) {
		selectionStart = 0;
		cursor = static_cast<uint32_t>(newValue.size());
	}
	if (oldCursor != cursor) {
		const auto [width, height] = text.getTextSize(newValue.substr(0, cursor));
		startToCursor = static_cast<float>(width);
	}
	// Check if the cursor is going out of bounds on the right
	const auto contentWidth = getContentRect().width();
	if (startToCursor - scroll > contentWidth) {
		scroll = startToCursor - contentWidth;
	}
	// Check if the cursor is going out of bounds on the left
	if (startToCursor - scroll < 0) {
		scroll = startToCursor;
	}

	if (oldValue != newValue) {
		text.setText(newValue);
		// if (onChange) onChange(newValue);
	}

	// Check if there is empty non whitespace space on the right of the text
	const auto &textSize = text.getSize();
	if (scroll + contentWidth > textSize.x) {
		scroll = textSize.x - contentWidth;
		scroll = scroll < 0 ? 0 : scroll;
	}

	if (selectionStart.has_value()) {
		const auto start = (std::min)(cursor, selectionStart.value());
		const auto end = (std::max)(cursor, selectionStart.value());

		if (start != oldSelectionStart || end != oldSelectionEnd) {
			const auto [width, height] = text.getTextSize(newValue.substr(0, start));
			startToSelection = static_cast<float>(width);
			const auto [width2, height2] = text.getTextSize(newValue.substr(start, end - start));
			selectedText.setText(newValue.substr(start, end - start));

			// Update selection size
			auto &selectionWidget = reinterpret_cast<Box::Impl &>(*children[1]);
			selectionWidget.data().sizeMode.width = static_cast<float>(width2);
		}
	}
}

void TextInput::Impl::onLayout(vec2 &maxSize, vec2 &minSize) {
	auto &children = getChildren();
	for (auto &child: children) {
		child->layout(maxSize.withX(std::numeric_limits<float>::max()));
	}
}

void TextInput::Impl::onArrange(vec2 &pos) {
	auto &widgetData = data();
	auto &children = getChildren();
	auto &textWidget = reinterpret_cast<Text::Impl &>(*children[0]);
	auto &selectionWidget = reinterpret_cast<Box::Impl &>(*children[1]);
	auto &selectedTextWidget = reinterpret_cast<Text::Impl &>(*children[2]);
	auto &cursorWidget = reinterpret_cast<Box::Impl &>(*children[3]);


	if (selectionStart.has_value()) {
		selectionWidget.data().visible = true;
		selectedTextWidget.data().visible = true;
	} else {
		selectionWidget.data().visible = false;
		selectedTextWidget.data().visible = false;
	}
	cursorWidget.data().visible = gd->active;

	const auto contentPos = pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset() - vec2{scroll, 0};

	textWidget.arrange(contentPos);
	selectionWidget.arrange(contentPos.withXOffset(startToSelection));
	selectedTextWidget.arrange(contentPos.withXOffset(startToSelection));
	cursorWidget.arrange(contentPos.withXOffset(startToCursor));
}

void TextInput::Impl::onDraw() {
	auto &children = getChildren();
	auto &textWidget = reinterpret_cast<Text::Impl &>(*children[0]);
	auto &selectionWidget = reinterpret_cast<Box::Impl &>(*children[1]);
	auto &selectedTextWidget = reinterpret_cast<Text::Impl &>(*children[2]);
	auto &cursorWidget = reinterpret_cast<Box::Impl &>(*children[3]);

	auto &renderer = Renderer::getInstance();
	renderer.addClipRect(getRect());

	textWidget.draw();
	selectionWidget.draw();
	selectedTextWidget.draw();
	cursorWidget.draw();

	renderer.popClipRect();
}
TextInput::operator Child() const {
	return Child{std::make_shared<Impl>(*this)};
}

void squi::TextInput::Impl::setActive(bool active) {
	gd->active = active;
}