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
	int64_t TextInput::State::indexFromPos(float x) const {
		if (!buffer.cachedLayoutPtr || buffer.cachedLayoutPtr->glyphs.empty()) return 0;

		auto it = std::lower_bound(buffer.cachedLayoutPtr->glyphs.begin(), buffer.cachedLayoutPtr->glyphs.end(), x, [](const TextLayout::Glyph &g, float val) {
			return (g.x + g.advance * 0.5f) < val;
		});

		if (it == buffer.cachedLayoutPtr->glyphs.end()) return buffer.cachedLayoutPtr->glyphs.back().byteOffset + 1;
		return it->byteOffset;
	}

	std::pair<int64_t, int64_t> TextInput::State::getWordRange(int64_t index) const {
		return buffer.getWordRange(index);
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
				buffer.cursor = newCursor;
			});
			if (!state.inputState || !state.inputState->isKeyDown(GestureKey::leftShift))
				buffer.selectionStart = std::nullopt;
			pivot = newCursor;
		} else if (clickCount == 2) {
			dragType = DragType::Word;
			auto range = getWordRange(newCursor);
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

	void TextInput::State::handleMouseDrag(const Gesture::State &state) {
		if (!state.renderObject) return;

		float localX = getRelativeCursorX(state);

		int64_t newPos = indexFromPos(localX);

		if (dragType == DragType::Char) {
			setState([&]() {
				if (!buffer.selectionStart.has_value()) buffer.selectionStart = pivot;
				buffer.cursor = newPos;
			});
		} else if (dragType == DragType::Word) {
			auto currentWord = getWordRange(newPos);
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

	Child TextInput::State::getSelectionBox(uint32_t widthToStart) const {
		if (!buffer.selectionStart.has_value()) return nullptr;
		auto selectionMin = buffer.getSelectionMin();
		auto selectionMax = buffer.getSelectionMax();
		widthToStart = static_cast<uint32_t>(buffer.cachedLayoutPtr->xForOffset(selectionMin));
		auto theme = Theme::of(element);
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
					.text = buffer.text->substr(selectionMin, selectionMax - selectionMin),
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
		auto widthToCursor = static_cast<uint32_t>(buffer.cachedLayoutPtr->xForOffset(buffer.cursor));

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
		if (manuallyScrolling) {
			minScroll = 0.f;
			maxScroll = std::max(cachedScrollData.contentMainAxis - cachedScrollData.viewMainAxis, 0.f);
		}
		minScroll = std::min(minScroll, maxScroll);
		scroll = std::clamp(scroll, minScroll, maxScroll);

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
							scroll -= state.getScroll().y * 40.f;
						}
						scroll -= state.getScroll().x * 40.f;
					});
				}

				auto &b = buffer;
				auto oldCursor = b.cursor;
				auto oldText = b.text;
				auto oldSelectionStart = b.selectionStart;

				b.clampCursors();
				b.handleTextInput(this->element->getApp()->inputState.g_textInput);
				b.handleBackspace(state);
				b.handleDelete(state);

				b.handleLeftArrow(state);
				b.handleRightArrow(state);

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
							.text = buffer.cachedLayoutPtr,
						},
						getSelectionBox(widthToCursor),
						getCursorBox(widthToCursor),
					},
				}},
			},
		};
	}
}// namespace squi
