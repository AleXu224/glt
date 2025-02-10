#include "textInput.hpp"
#include "box.hpp"
#include "cstdint"
#include "gestureDetector.hpp"
#include "text.hpp"
#include "textData.hpp"
#include "vec2.hpp"
#include "widget.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <format>
#include <memory>
#include <optional>


using namespace squi;

TextInput::Impl::Impl(const TextInput &args)
	: Widget(args.widget, Widget::FlagsArgs::Default()),
	  textWidget(Text{
		  .text{args.text},
		  .fontSize = args.fontSize,
		  .lineWrap = false,
		  .font{args.font},
		  .color{args.color},
	  }),
	  selectionWidget(Box{
		  .widget{.width = 0.f},
		  .color{0x0078D4FF},
	  }),
	  selectionTextWidget(Text{
		  .text{""},
		  .fontSize = args.fontSize,
		  .lineWrap = false,
		  .font{args.font},
		  .color{0xFFFFFFFF},
	  }),
	  cursorWidget(Box{
		  .widget{
			  .width = 2.f,
		  },
		  .color{0xFFFFFFFF},
	  }),
	  onTextChanged(args.onTextChanged),
	  setActiveObs(args.controller.setActive.observe([&self = *this](bool newVal) {
		  self.setActive(newVal);
	  })),
	  selectAllObs(args.controller.selectAll.observe([&self = *this]() {
		  self.selectionStart = 0;
		  const auto textValue = self.textWidget->as<Text::Impl>().getText();
		  self.cursor = static_cast<int64_t>(textValue.size());

		  self.updateSelection();
	  })) {
	addChild(textWidget);
	addChild(selectionWidget);
	addChild(selectionTextWidget);
	addChild(cursorWidget);
	cursorWidget->flags.visible = false;
}

void TextInput::Impl::clampCursors() {
	const auto text = getText();
	cursor = std::clamp(cursor, static_cast<int64_t>(0), static_cast<int64_t>(text.size()));
	if (selectionStart.has_value()) {
		selectionStart = std::clamp(selectionStart.value_or(0), static_cast<int64_t>(0), static_cast<int64_t>(text.size()));
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
	return textWidget->as<Text::Impl>().getText();
}

void TextInput::Impl::setText(std::string_view text) {
	if (selectionStart.has_value()) {
		selectionStart = std::nullopt;
		selectionWidget->flags.visible = false;
		selectionTextWidget->flags.visible = false;
		selectionTextWidget->as<Text::Impl>().setText("");
	}
	textWidget->as<Text::Impl>().setText(text);
	if (onTextChanged) onTextChanged(getText());
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
	if (!active) {
		cursorWidget->flags.visible = false;
		return;
	}
	cursorWidget->flags.visible = true;
	handleMouseInput();

	auto &text = textWidget->as<Text::Impl>();
	const auto oldCursor = cursor;
	const auto oldSelectionStart = selectionStart;

	clampCursors();

	auto &inputState = InputState::of(this);
	if (!inputState.g_textInput.empty()) {
		clearSelection();
		const auto textValue = text.getText();
		setText(std::format("{}{}{}", textValue.substr(0, cursor), inputState.g_textInput, textValue.substr(cursor)));
		cursor += static_cast<int64_t>(inputState.g_textInput.size());
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_BACKSPACE)) {
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
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_DELETE)) {
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
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_LEFT)) {
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
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_RIGHT)) {
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
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_HOME)) {
		if (key->mods & GLFW_MOD_SHIFT && cursor > 0 && !selectionStart.has_value())
			selectionStart = cursor;
		else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value())
			selectionStart = std::nullopt;

		cursor = std::min<int64_t>(cursor, 0);
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_END)) {
		const auto textValue = text.getText();
		if (key->mods & GLFW_MOD_SHIFT && cursor < static_cast<int64_t>(textValue.size()) && !selectionStart.has_value())
			selectionStart = cursor;
		else if (!(key->mods & GLFW_MOD_SHIFT) && selectionStart.has_value())
			selectionStart = std::nullopt;

		cursor = std::max(cursor, static_cast<int64_t>(textValue.size()));
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_ESCAPE)) {
		selectionStart = std::nullopt;
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_A)) {
		if (key->mods & GLFW_MOD_CONTROL) {
			selectionStart = 0;
			const auto textValue = text.getText();
			cursor = static_cast<int64_t>(textValue.size());
		}
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_C)) {
		if (key->mods & GLFW_MOD_CONTROL && selectionStart.has_value()) {
			const std::string textToCopy = std::string(getText().substr(getSelectionMin(), getSelectionMax() - getSelectionMin()));
			glfwSetClipboardString(nullptr, textToCopy.c_str());
		}
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_X)) {
		if (key->mods & GLFW_MOD_CONTROL && selectionStart.has_value()) {
			const std::string textToCopy = std::string(getText().substr(getSelectionMin(), getSelectionMax() - getSelectionMin()));
			glfwSetClipboardString(nullptr, textToCopy.c_str());
			clearSelection();
		}
	} else if (const auto key = inputState.getKeyPressedOrRepeat(GLFW_KEY_V)) {
		if (key->mods & GLFW_MOD_CONTROL) {
			const auto *const clipboardText = glfwGetClipboardString(nullptr);
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
		updateSelection();
	}
}

vec2 TextInput::Impl::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink /*shouldShrink*/, bool final) {
	auto &children = getChildren();
	// A text input can scroll horizontally so it has no real maximum width
	maxSize = vec2::infinity().withY(static_cast<float>(textWidget->as<Text::Impl>().getLineHeight()));
	vec2 ret{};
	for (auto &child: children) {
		const auto size = child->layout(maxSize, minSize, {}, final);
		ret.x = std::max(size.x, ret.x);
		ret.y = std::max(size.y, ret.y);
	}

	return ret;
}

void TextInput::Impl::arrangeChildren(vec2 &pos) {
	const auto text = getText();
	const auto [startToCursorWidth, startToCursorHeight] = textWidget->as<Text::Impl>().getTextSize(text.substr(0, cursor));
	const auto contentWidth = std::max(getContentRect().width(), 0.f);
	scroll = std::clamp(scroll, static_cast<float>(startToCursorWidth) - contentWidth, static_cast<float>(startToCursorWidth));
	// Avoid empty space on the right side when the text size is bigger than the input size
	scroll = std::clamp(scroll, 0.f, std::max(textWidget->getSize().x - contentWidth, 0.f));

	const auto lineHeight = textWidget->as<Text::Impl>().getLineHeight();
	const auto verticalOffset = std::round((getContentSize().y - static_cast<float>(lineHeight)) / 2.f);
	const auto contentPos = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset() - vec2{scroll, -verticalOffset};

	textWidget->arrange(contentPos);
	if (selectionStart.has_value()) {
		const auto [width, height] = textWidget->as<Text::Impl>().getTextSize(text.substr(0, getSelectionMin()));
		selectionWidget->arrange(contentPos.withXOffset(static_cast<float>(width)));
		selectionTextWidget->arrange(contentPos.withXOffset(static_cast<float>(width)));
	}
	cursorWidget->arrange(contentPos.withXOffset(static_cast<float>(startToCursorWidth)));
}

void TextInput::Impl::drawChildren() {
	auto &instance = Window::of(this).engine.instance;
	instance.pushScissor(getRect());

	textWidget->draw();
	selectionWidget->draw();
	selectionTextWidget->draw();
	cursorWidget->draw();

	instance.popScissor();
}
TextInput::operator Child() const {
	return GestureDetector{.child{std::make_unique<Impl>(*this)}};
}

void squi::TextInput::Impl::setActive(bool active) {
	if (!active) {
		selectionStart = std::nullopt;
		updateSelection();
	}
	this->active = active;
}

void squi::TextInput::Impl::handleMouseInput() {
	if (!GestureDetector::canClick(*this)) return;
	auto &inputState = InputState::of(this);
	if (!inputState.isKey(GLFW_MOUSE_BUTTON_1, GLFW_PRESS)) return;
	auto &textWidget = this->textWidget->as<Text::Impl>();
	const auto &quads = textWidget.getData().quads;
	const auto mousePos = inputState.getMousePos();
	const auto lineHeight = textWidget.getLineHeight();
	const auto textPos = textWidget.getPos();

	// No text, nothing to click
	if (quads.empty()) return;

	auto lineIt = std::lower_bound(quads.begin(), quads.end(), mousePos.y, [&](const std::vector<Engine::TextQuad> &vec, float yPos) {
		return textPos.y + static_cast<float>((std::distance(&quads.front(), &vec) + 1) * lineHeight) < yPos;
	});

	if (lineIt == quads.end()) {
		// Select the last line if the user clicks under the text but still within the input
		lineIt--;
	}

	const auto quadIt = std::upper_bound(lineIt->begin(), lineIt->end(), mousePos.x, [](float xPos, const Engine::TextQuad &quad) {
		return xPos < (quad.getPos().x + quad.getOffset().x + quad.getSize().x / 2);
	});

	if (selectionStart.has_value()) {
		selectionStart = std::nullopt;
		selectionWidget->flags.visible = false;
		selectionTextWidget->flags.visible = false;
		selectionTextWidget->as<Text::Impl>().setText("");
	}
	cursor = std::distance(lineIt->begin(), quadIt);
	clampCursors();
	reArrange();
}

void squi::TextInput::Impl::updateSelection() {
	if (selectionStart.has_value()) {
		auto &text = textWidget->as<Text::Impl>();
		selectionWidget->flags.visible = true;
		selectionTextWidget->flags.visible = true;
		auto textVal = getText();
		const auto newText = textVal.substr(getSelectionMin(), getSelectionMax() - getSelectionMin());
		const auto [width, height] = text.getTextSize(newText);
		selectionWidget->state.width = static_cast<float>(width);
		selectionTextWidget->as<Text::Impl>().setText(newText);
	} else {
		selectionWidget->flags.visible = false;
		selectionTextWidget->flags.visible = false;
		selectionTextWidget->as<Text::Impl>().setText("");
	}
}

void squi::TextInput::Impl::setColor(const Color &newColor) {
	textWidget->as<Text::Impl>().setColor(newColor);
}
