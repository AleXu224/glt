#include "widgets/textArea.hpp"

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
	void TextArea::State::initState() {
		controller = widget->controller;
		buffer.text = &controller.controlBlock->text;
		buffer.onTextChanged = [this](const std::string &text) {
			buffer.regenerateLayout(font);
			controller.notifyTextChanged();
			if (widget->onTextChanged) widget->onTextChanged(text);
		};
		textObserver = controller.getTextObserver([this](const std::string &newText) {
			buffer.regenerateLayout(font);
			buffer.clampCursors();
		});
		buffer.regenerateLayout(font);
	}

	void TextArea::State::widgetUpdated() {
		if (controller == widget->controller) return;
		controller = widget->controller;
		buffer.text = &controller.controlBlock->text;
		buffer.onTextChanged = [this](const std::string &text) {
			buffer.regenerateLayout(font);
			controller.notifyTextChanged();
			if (widget->onTextChanged) widget->onTextChanged(text);
		};
		textObserver = controller.getTextObserver([this](const std::string &newText) {
			buffer.regenerateLayout(font);
			buffer.clampCursors();
		});
	}

	int64_t TextArea::State::indexFromPos(float x, float y) const {
		if (!buffer.cachedLayoutPtr) return 0;

		uint32_t targetLine = 0;
		if (buffer.cachedLayoutPtr->lineHeight > 0.f) {
			targetLine = static_cast<uint32_t>(std::max(0.f, y / buffer.cachedLayoutPtr->lineHeight));
		}

		if (buffer.cachedLayoutPtr->glyphs.empty()) {
			if (targetLine == 0) return 0;
			if (targetLine <= buffer.cachedLayoutPtr->newlineOffsets.size())
				return buffer.cachedLayoutPtr->newlineOffsets[targetLine - 1] + 1;
			return static_cast<int64_t>(buffer.text->size());
		}

		auto it = std::lower_bound(buffer.cachedLayoutPtr->glyphs.begin(), buffer.cachedLayoutPtr->glyphs.end(), targetLine, [](const TextLayout::Glyph &g, uint32_t line) {
			return g.lineIndex < line;
		});

		if (it == buffer.cachedLayoutPtr->glyphs.end() || it->lineIndex != targetLine) {
			if (it == buffer.cachedLayoutPtr->glyphs.begin()) return 0;
			return buffer.cachedLayoutPtr->glyphs.back().byteOffset;
		}

		auto end = it;
		while (end != buffer.cachedLayoutPtr->glyphs.end() && end->lineIndex == targetLine) ++end;

		auto best = std::lower_bound(it, end, x, [](const TextLayout::Glyph &g, float val) {
			return (g.x + g.advance * 0.5f) < val;
		});

		if (best == end) --best;
		return best->byteOffset;
	}

	float TextArea::State::getRelativeCursorX(const Gesture::State &state) const {
		float paddingLeft = widget->widget.padding.value_or(Padding{}).left;
		float marginLeft = widget->widget.margin.value_or(Margin{}).left;
		return state.getCursorPos().x - state.renderObject->getRect().left + scrollX - paddingLeft - marginLeft;
	}

	float TextArea::State::getRelativeCursorY(const Gesture::State &state) const {
		float paddingTop = widget->widget.padding.value_or(Padding{}).top;
		float marginTop = widget->widget.margin.value_or(Margin{}).top;
		return state.getCursorPos().y - state.renderObject->getRect().top + scrollY - paddingTop - marginTop;
	}

	void TextArea::State::handleMousePress(const Gesture::State &state) {
		if (!state.renderObject) return;

		float localX = getRelativeCursorX(state);
		float localY = getRelativeCursorY(state);

		int64_t newCursor = indexFromPos(localX, localY);

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
				buffer.cursor = newCursor;
			});
			if (!state.inputState || !state.inputState->isKeyDown(GestureKey::leftShift))
				buffer.selectionStart = std::nullopt;
			pivot = newCursor;
		} else if (clickCount == 2) {
			dragType = DragType::Word;
			auto range = buffer.getWordRange(newCursor);
			pivotRange = range;
			setState([&]() {
				buffer.selectionStart = range.first;
				buffer.cursor = range.second;
			});
		} else if (clickCount == 3) {
			dragType = DragType::Line;
			pivotRange = {0, static_cast<int64_t>(buffer.text->size())};
			setState([&]() {
				buffer.selectionStart = 0;
				buffer.cursor = static_cast<int64_t>(buffer.text->size());
			});
		}
	}

	void TextArea::State::handleMouseDrag(const Gesture::State &state) {
		if (!state.renderObject) return;

		float localX = getRelativeCursorX(state);
		float localY = getRelativeCursorY(state);

		int64_t newPos = indexFromPos(localX, localY);

		if (dragType == DragType::Char) {
			setState([&]() {
				if (!buffer.selectionStart.has_value()) buffer.selectionStart = pivot;
				buffer.cursor = newPos;
			});
		} else if (dragType == DragType::Word) {
			auto currentWord = buffer.getWordRange(newPos);
			setState([&]() {
				if (newPos >= pivotRange.second) {
					buffer.selectionStart = pivotRange.first;
					buffer.cursor = currentWord.second;
				} else if (newPos <= pivotRange.first) {
					buffer.selectionStart = pivotRange.second;
					buffer.cursor = currentWord.first;
				} else {
					buffer.selectionStart = pivotRange.first;
					buffer.cursor = pivotRange.second;
				}
			});
		}
	}

	Child TextArea::State::buildSelectionBoxes() const {
		if (!buffer.selectionStart.has_value()) return nullptr;
		auto selMin = buffer.getSelectionMin();
		auto selMax = buffer.getSelectionMax();
		if (selMin == selMax) return nullptr;
		if (!buffer.cachedLayoutPtr || buffer.cachedLayoutPtr->glyphs.empty()) return nullptr;

		Children boxes{};
		auto theme = Theme::of(element);

		auto it = std::lower_bound(buffer.cachedLayoutPtr->glyphs.begin(), buffer.cachedLayoutPtr->glyphs.end(), selMin, [](const TextLayout::Glyph &g, int64_t offset) {
			return g.byteOffset < offset;
		});
		if (it == buffer.cachedLayoutPtr->glyphs.end()) return nullptr;

		while (it != buffer.cachedLayoutPtr->glyphs.end() && it->byteOffset < selMax) {
			uint32_t line = it->lineIndex;
			float yOffset = static_cast<float>(line) * buffer.cachedLayoutPtr->lineHeight;

			int64_t lineSelStart = it->byteOffset;
			float xStart = it->x;

			it = std::lower_bound(it, buffer.cachedLayoutPtr->glyphs.end(), line, [&selMax](const TextLayout::Glyph &g, uint32_t line) {
				return (g.byteOffset < selMax && g.lineIndex <= line) || (g.lineIndex == (line + 1) && g.isNewline() && g.byteOffset <= selMax);
			});

			int64_t lineSelEnd;
			if (it == buffer.cachedLayoutPtr->glyphs.end()) {
				lineSelEnd = static_cast<int64_t>(buffer.text->size());
			} else {
				lineSelEnd = it->byteOffset;
			}

			boxes.emplace_back(Offset{
				.calculateContentBounds = [xStart, yOffset](const Rect &bounds, const SingleChildRenderObject &) -> Rect {
					auto ret = bounds;
					ret.offset({xStart, yOffset});
					return ret;
				},
				.child = Box{
					.widget{
						.width = Size::Wrap,
						.height = Size::Wrap,
					},
					.color = theme.accent,
					.child = Text{
						.text = buffer.text->substr(lineSelStart, lineSelEnd - lineSelStart),
						.color = theme.accent.isLight() ? Color::black : Color::white,
					},
				},
			});
		}

		if (boxes.empty()) return nullptr;
		return Stack{
			.widget{
				.width = Size::Wrap,
				.height = Size::Wrap,
			},
			.children = std::move(boxes),
		};
	}

	Child TextArea::State::buildCursorBox() const {
		if (!widget->active) return nullptr;
		if (!buffer.cachedLayoutPtr) return nullptr;

		float cursorX = buffer.cachedLayoutPtr->xForOffset(buffer.cursor);
		uint32_t cursorLine = buffer.cachedLayoutPtr->lineForOffset(buffer.cursor);
		float cursorY = static_cast<float>(cursorLine) * buffer.cachedLayoutPtr->lineHeight;

		return Offset{
			.calculateContentBounds = [cursorX, cursorY](const Rect &bounds, const SingleChildRenderObject &) -> Rect {
				auto ret = bounds;
				ret.offset({cursorX, cursorY});
				return ret;
			},
			.child = Box{
				.widget{
					.width = 1.f,
					.height = buffer.cachedLayoutPtr->lineHeight,
				},
			},
		};
	}

	Child TextArea::State::build(const Element &) {
		auto constraints = widget->widget.sizeConstraints.value_or(BoxConstraints{
			.minWidth = 100.f,
		});

		float availableWidth = constraints.maxWidth;
		if (availableWidth <= 0.f) availableWidth = std::numeric_limits<float>::max();

		this->element->addPostLayoutTask([this]() {
			if (this->cachedVerticalScrollData != *this->verticalScrollController) {
				this->cachedVerticalScrollData = *this->verticalScrollController;
				this->element->markNeedsRebuild();
			}
			if (this->cachedHorizontalScrollData != *this->horizontalScrollController) {
				this->cachedHorizontalScrollData = *this->horizontalScrollController;
				this->element->markNeedsRebuild();
			}
		});

		auto args = widget->widget;
		args.height = args.height.value_or(Size::Shrink);
		args.padding = args.padding.value_or(Padding{}.withRight(1.f));

		auto buildInnerStack = [&]() -> Child {
			return Stack{
				.widget{
					.width = Size::Wrap,
					.height = Size::Wrap,
				},
				.children{
					Text{
						.text = buffer.cachedLayoutPtr,
					},
					buildSelectionBoxes(),
					buildCursorBox(),
				},
			};
		};

		Child content = buildInnerStack();

		float cursorY = static_cast<float>(buffer.cachedLayoutPtr->lineForOffset(buffer.cursor)) * buffer.cachedLayoutPtr->lineHeight;
		float minScrollV = std::max(cursorY + buffer.cachedLayoutPtr->lineHeight - cachedVerticalScrollData.viewMainAxis, 0.f);
		float maxScrollV = std::min(cursorY, std::max(cachedVerticalScrollData.contentMainAxis - cachedVerticalScrollData.viewMainAxis, 0.f));
		if (manuallyScrolling) {
			minScrollV = 0.f;
			maxScrollV = std::max(cachedVerticalScrollData.contentMainAxis - cachedVerticalScrollData.viewMainAxis, 0.f);
		}
		minScrollV = std::min(minScrollV, maxScrollV);
		scrollY = std::clamp(scrollY, minScrollV, maxScrollV);

		float cursorX = buffer.cachedLayoutPtr->xForOffset(buffer.cursor);
		float minScrollH = std::max(cursorX + buffer.cachedLayoutPtr->lineHeight - cachedHorizontalScrollData.viewMainAxis, 0.f);
		float maxScrollH = std::min(cursorX, std::max(cachedHorizontalScrollData.contentMainAxis - cachedHorizontalScrollData.viewMainAxis, 0.f));
		if (manuallyScrolling) {
			minScrollH = 0.f;
			maxScrollH = std::max(cachedHorizontalScrollData.contentMainAxis - cachedHorizontalScrollData.viewMainAxis, 0.f);
		}
		minScrollH = std::min(minScrollH, maxScrollH);
		scrollX = std::clamp(scrollX, minScrollH, maxScrollH);

		content = Scrollable{
			.widget = args,
			.direction = Axis::Horizontal,
			.scroll = scrollX,
			.controller = horizontalScrollController,
			.children{
				Scrollable{
					.direction = Axis::Vertical,
					.scroll = scrollY,
					.controller = verticalScrollController,
					.children{std::move(content)},
				},
			},
		};

		return Gesture{
			.onPress = [this](const Gesture::State &state) {
				handleMousePress(state);
			},
			.onDrag = [this](const Gesture::State &state) {
				auto oldCursor = buffer.cursor;
				handleMouseDrag(state);
				if (oldCursor != buffer.cursor)
					setState([&]() {
						manuallyScrolling = false;
					});
			},
			.onUpdate = [this](const Gesture::State &state) {
				if (!widget->active)
					return;

				if (state.getScroll().length() > 0.f) {
					setState([&]() {
						manuallyScrolling = true;
						if (state.inputState->isKeyDown(GestureKey::leftShift) || state.inputState->isKeyDown(GestureKey::rightShift)) {
							scrollX -= state.getScroll().y * 40.f;
						} else {
							scrollY -= state.getScroll().y * 40.f;
						}
						scrollX -= state.getScroll().x * 40.f;
					});
				}

				auto &b = buffer;
				auto oldCursor = b.cursor;
				auto oldText = b.text;
				auto oldSelectionStart = b.selectionStart;

				b.clampCursors();
				b.handleTextInput(this->element->getApp()->inputState.g_textInput);
				b.handleEnter(state);
				b.handleBackspace(state);
				b.handleDelete(state);

				b.handleLeftArrow(state);
				b.handleRightArrow(state);
				b.handleUpArrow(state);
				b.handleDownArrow(state);

				b.handleHome(state);
				b.handleEnd(state);

				b.handleEscape(state);
				b.handleSelectAll(state);
				b.handleCopy(state);
				b.handleCut(state);
				b.handlePaste(state);

				if (oldText != b.text || oldCursor != b.cursor || oldSelectionStart != b.selectionStart)
					setState();

				if (oldCursor != b.cursor) {
					this->element->markNeedsReposition();
					setState([&]() {
						manuallyScrolling = false;
					});
				}
			},
			.child = std::move(content),
		};
	}
}// namespace squi
