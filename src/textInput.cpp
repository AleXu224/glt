#include "textInput.hpp"
#include "box.hpp"
#include "cstdint"
#include "fontStore.hpp"
#include "renderer.hpp"
#include "text.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <optional>


using namespace squi;

TextInput::Impl::Impl(const TextInput &args)
	: Widget(args.widget, Widget::Options{
							  .shouldDrawChildren = false,
						  }) {
	addChild(Text{
		.text{"scjsnjcnaskjcas"},
	});
	// Selection
	addChild(Box{
		.widget{
			.sizeBehavior{
				.vertical = SizeBehaviorType::FillParent,
			},
		},
		.color{Color::HEX(0x0022FF80)},
	});
	// Cursor
	addChild(Box{
		.widget{
			.size{2},
			.sizeBehavior{
				.vertical = SizeBehaviorType::FillParent,
			},
		},
		.color{Color::HEX(0xFF0000FF)},
	});
}

// TODO: Fix the horrible performance of this
void TextInput::Impl::onUpdate() {
	auto &widgetData = data();
	auto &gd = widgetData.gestureDetector;
	if (!gd.active) return;

	auto &children = getChildren();
	auto &text = reinterpret_cast<Text::Impl &>(*children[0]);

	const auto oldValue = text.getText();
	auto newValue = std::string(oldValue);

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
		cursor = static_cast<int>(newValue.size());
	}
	const auto [width, height] = text.getTextSize(newValue.substr(0, cursor));
	const auto cursorXPos = static_cast<float>(width);
	// Check if the cursor is going out of bounds on the right
	if (cursorXPos - scroll > widgetData.size.x) {
		scroll = cursorXPos - widgetData.size.x;
	}
	// Check if the cursor is going out of bounds on the left
	if (cursorXPos - scroll < 0) {
		scroll = cursorXPos;
	}
	// Check if there is empty non whitespace space on the right of the text
	const auto [newWidth, newHeight] = text.getTextSize(newValue);
	if (scroll + widgetData.size.x > static_cast<float>(newWidth)) {
		scroll = static_cast<float>(newWidth) - widgetData.size.x;
		scroll = scroll < 0 ? 0 : scroll;
	}


	if (oldValue != newValue) {
		text.setText(newValue);
		// if (onChange) onChange(newValue);
	}
}

void TextInput::Impl::onDraw() {
	auto &widgetData = data();
	auto &children = getChildren();
	auto &textWidget = reinterpret_cast<Text::Impl &>(*children[0]);
	auto &selectionWidget = reinterpret_cast<Box::Impl &>(*children[1]);
	auto &cursorWidget = reinterpret_cast<Box::Impl &>(*children[2]);

	auto &renderer = Renderer::getInstance();
	renderer.addClipRect(getRect());

	const auto &pos = widgetData.pos + widgetData.margin.getPositionOffset() + widgetData.padding.getPositionOffset() - vec2{scroll, 0};

	if (selectionStart.has_value()) {
		const uint32_t selectStartPos = (std::min)(cursor, selectionStart.value());
		const uint32_t selectEndPos = (std::max)(cursor, selectionStart.value());

		const auto [width, height] = textWidget.getTextSize(textWidget.getText().substr(0, selectStartPos));
		auto &selectionData = selectionWidget.data();
		selectionData.pos = pos.withXOffset(static_cast<float>(width));
		const auto [selectionWidth, selectionHeight] = textWidget.getTextSize(textWidget.getText().substr(selectStartPos, selectEndPos - selectStartPos));
		selectionData.size.x = static_cast<float>(selectionWidth);
		selectionData.visible = true;
	} else {
		selectionWidget.data().visible = false;
	}

	textWidget.data().pos = pos;
	const auto [width, height] = textWidget.getTextSize(textWidget.getText().substr(0, cursor));
	cursorWidget.data().pos = pos.withXOffset(static_cast<float>(width));
	cursorWidget.data().visible = widgetData.gestureDetector.active;

	textWidget.draw();
	selectionWidget.draw();
	cursorWidget.draw();

	renderer.popClipRect();
}