#include "widget.hpp"
#include "ranges"
#include "window.hpp"
#include <algorithm>
#include <memory>
#include <optional>


using namespace squi;

uint64_t Widget::idCounter = 1;// 0 is reserved for exceptions
uint32_t Widget::widgetCount = 0;

Widget::Widget(const Args &args, const FlagsArgs &flags)
	: flags(this, flags),
	  state{
		  .width{this, args.width.value_or(Size::Expand)},
		  .height{this, args.height.value_or(Size::Expand)},
		  .sizeConstraints = args.sizeConstraints,
		  .margin{this, args.margin.value_or(Margin{})},
		  .padding{this, args.padding.value_or(Margin{})},
		  .parent{this, nullptr},
		  .root{this, nullptr},
	  },
	  id(idCounter++) {
	if (args.onInit) m_funcs.onInit.push_back(args.onInit);
	if (args.onUpdate) m_funcs.onUpdate.push_back(args.onUpdate);
	if (args.afterUpdate) m_funcs.afterUpdate.push_back(args.afterUpdate);
	if (args.beforeLayout) m_funcs.beforeLayout.push_back(args.beforeLayout);
	if (args.onLayout) m_funcs.onLayout.push_back(args.onLayout);
	if (args.afterLayout) m_funcs.afterLayout.push_back(args.afterLayout);
	if (args.onArrange) m_funcs.onArrange.push_back(args.onArrange);
	if (args.beforeDraw) m_funcs.beforeDraw.push_back(args.beforeDraw);
	if (args.onDraw) m_funcs.onDraw.push_back(args.onDraw);
	if (args.afterDraw) m_funcs.afterDraw.push_back(args.afterDraw);
	if (args.onChildAdded) m_funcs.onChildAdded.push_back(args.onChildAdded);
	if (args.onChildRemoved) m_funcs.onChildRemoved.push_back(args.onChildRemoved);
#ifndef NDEBUG
	if (args.onDebugUpdate) m_funcs.onDebugUpdate.push_back(args.onDebugUpdate);
	if (args.onDebugLayout) m_funcs.onDebugLayout.push_back(args.onDebugLayout);
	if (args.onDebugArrange) m_funcs.onDebugArrange.push_back(args.onDebugArrange);
	if (args.onDebugDraw) m_funcs.onDebugDraw.push_back(args.onDebugDraw);
#endif
	widgetCount++;
}

Widget::~Widget() {
	widgetCount--;

	// if (auto it = Store::widgets.find(id); it != Store::widgets.end()) {
	// 	Store::widgets.erase(it);
	// }
}

Widget::FunctionArgs &Widget::funcs() {
	return m_funcs;
}

const Widget::FunctionArgs &Widget::funcs() const {
	return m_funcs;
}

std::vector<Child> &Widget::getChildren() {
	return children;
}

const std::vector<Child> &Widget::getChildren() const {
	return children;
}

std::vector<Rect> Widget::getHitcheckRect() const {
	if (flags.isInteractive && *flags.visible)
		return {getRect()};
	else
		return {};
}

void Widget::setChildren(const Children &newChildren) {
	for (auto &child: children) {
		for (auto &func: m_funcs.onChildRemoved) {
			if (func) func(*this, child);
		}
	}
	children.clear();
	for (auto &child: newChildren) {
		if (child) {
			child->state.parent = this;
			if (!child->initialized) child->initialize();
			children.push_back(child);
			for (auto &func: m_funcs.onChildAdded) {
				if (func) func(*this, child);
			}
		}
	}
	reLayout();
}

void Widget::addChild(const Child &child) {
	if (child) {
		child->state.parent = this;
		if (!child->initialized) child->initialize();
		children.push_back(child);
		for (auto &func: m_funcs.onChildAdded) {
			if (func) func(*this, child);
		}
		reLayout();
	}
}

void Widget::updateChildren() {
	for (auto &child: std::views::reverse(children)) {
		child->state.parent = this;
		child->state.root = *state.root;
		child->update();
	}
}

void Widget::update() {
#ifndef NDEBUG
	for (auto &func: m_funcs.onDebugUpdate) {
		if (func) func();
	}
#endif

	// On update
	for (auto &func: m_funcs.onUpdate) {
		if (func) func(*this);
	}
	onUpdate();

	// Update the children
	if (flags.shouldUpdateChildren && *flags.visible) {
		updateChildren();
	}
	children.erase(
		std::remove_if(children.begin(), children.end(), [&](const Child &child) -> bool {
			if (child->shouldDelete) {
				for (auto &func: m_funcs.onChildRemoved) {
					if (func) func(*this, child);
				}
				reLayout();
			}
			return child->shouldDelete;
		}),
		children.end());

	// After update
	for (auto &func: m_funcs.afterUpdate) {
		if (func) func(*this);
	}
	afterUpdate();
}

vec2 Widget::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink) {
	for (auto &child: children) {
		const auto size = child->layout(maxSize, minSize, shouldShrink);
		minSize.x = std::clamp(size.x, minSize.x, maxSize.x);
		minSize.y = std::clamp(size.y, minSize.y, maxSize.y);
	}

	return minSize;
}

vec2 squi::Widget::layout(vec2 maxSize, vec2 minSize, ShouldShrink forceShrink) {
#ifndef NDEBUG
	for (auto &func: m_funcs.onDebugLayout) {
		if (func) func();
	}
#endif

	for (auto &func: m_funcs.beforeLayout) {
		if (func) func(*this, maxSize, minSize);
	}

	// An invisible widget has no size
	if (!*flags.visible) return {0, 0};

	const auto &constraints = state.sizeConstraints;

	const auto margin = state.margin->getSizeOffset();
	const auto padding = state.padding->getSizeOffset();

	maxSize.x = std::max(margin.x + padding.x, maxSize.x);
	maxSize.y = std::max(margin.y + padding.y, maxSize.y);

	maxSize -= margin;

	// Handle the max size constraints
	if (constraints.maxWidth.has_value()) {
		maxSize.x = std::min(maxSize.x, *constraints.maxWidth);
	}
	if (constraints.maxHeight.has_value()) {
		maxSize.y = std::min(maxSize.y, *constraints.maxHeight);
	}

	ShouldShrink shouldShrink = forceShrink;

	// Handle the size mode constraints
	switch (state.width->index()) {
		case 0: {
			minSize.x = padding.x;
			maxSize.x = std::clamp(std::get<0>(*state.width), minSize.x, maxSize.x);
			shouldShrink.width = false;
			break;
		}
		case 1: {
			const auto &size = std::get<1>(*state.width);
			if (size == Size::Shrink) {
				shouldShrink.width = true;
				minSize.x = padding.x;
			} else {
				minSize.x = std::clamp(minSize.x - margin.x, padding.x, maxSize.x);
			}
		}
	}

	switch (state.height->index()) {
		case 0: {
			minSize.y = padding.y;
			maxSize.y = std::clamp(std::get<0>(*state.height), minSize.y, maxSize.y);
			shouldShrink.height = false;
			break;
		}
		case 1: {
			const auto &size = std::get<1>(*state.height);
			if (size == Size::Shrink) {
				shouldShrink.height = true;
				minSize.y = padding.y;
			} else {
				minSize.y = std::clamp(minSize.y - margin.y, padding.y, maxSize.y);
			}
		}
	}

	// Handle the min size constraints
	if (constraints.minWidth.has_value()) {
		minSize.x = std::clamp(*constraints.minWidth, minSize.x, maxSize.x);
	}
	if (constraints.minHeight.has_value()) {
		minSize.y = std::clamp(*constraints.minHeight, minSize.y, maxSize.y);
	}

	for (auto &func: m_funcs.onLayout) {
		if (func) func(*this, maxSize, minSize);
	}
	onLayout(maxSize, minSize);

	if (flags.shouldLayoutChildren) {
		const auto contentSize = layoutChildren(maxSize - padding, minSize - padding, shouldShrink);
		minSize.x = std::clamp(contentSize.x + padding.x, minSize.x, maxSize.x);
		minSize.y = std::clamp(contentSize.y + padding.y, minSize.y, maxSize.y);
	}

	switch (state.width->index()) {
		case 0: {
			size.x = std::clamp(std::get<0>(*state.width), minSize.x, maxSize.x);
			break;
		}
		case 1: {
			switch (std::get<1>(*state.width)) {
				case Size::Expand: {
					if (shouldShrink.width)
						size.x = minSize.x;
					else
						size.x = maxSize.x;
					break;
				}
				case Size::Shrink: {
					size.x = minSize.x;
					break;
				}
			}
		}
	}
	switch (state.height->index()) {
		case 0: {
			size.y = std::clamp(std::get<0>(*state.height), minSize.y, maxSize.y);
			break;
		}
		case 1: {
			switch (std::get<1>(*state.height)) {
				case Size::Expand: {
					if (shouldShrink.height)
						size.y = minSize.y;
					else
						size.y = maxSize.y;
					break;
				}
				case Size::Shrink: {
					size.y = minSize.y;
					break;
				}
			}
		}
	}

	for (auto &func: m_funcs.afterLayout) {
		if (func) func(*this, maxSize, minSize);
	}
	postLayout(size);

	return size + state.margin->getSizeOffset();
}

void Widget::arrangeChildren(vec2 &pos) {
	const auto childPos = pos + state.margin->getPositionOffset() + state.padding->getPositionOffset();
	for (auto &child: children) {
		child->arrange(childPos);
	}
}

void Widget::arrange(vec2 pos) {
#ifndef NDEBUG
	for (auto &func: m_funcs.onDebugArrange) {
		if (func) func();
	}
#endif

	if (!*flags.visible) return;
	for (auto &onArrange: m_funcs.onArrange) {
		if (onArrange) onArrange(*this, pos);
	}
	onArrange(pos);
	this->pos = pos;

	if (flags.shouldArrangeChildren) {
		arrangeChildren(pos);
	}
	postArrange(pos);
}

void Widget::drawChildren() {
	for (auto &child: children) {
		child->draw();
	}
}

void Widget::draw() {
#ifndef NDEBUG
	for (auto &func: m_funcs.onDebugDraw) {
		if (func) func();
	}
#endif

	for (auto &func: m_funcs.beforeDraw) {
		if (func) func(*this);
	}
	if (!*flags.visible) return;
	for (auto &func: m_funcs.onDraw) {
		if (func) func(*this);
	}
	onDraw();

	if (*flags.shouldDrawChildren) {
		drawChildren();
	}

	for (auto &func: m_funcs.afterDraw) {
		if (func) func(*this);
	}
}

void Widget::initialize() {
	for (auto &func: m_funcs.onInit) {
		if (func) func(*this);
	}
}

void Widget::reDraw() const {
	if (!*state.root) return;
	auto *window = reinterpret_cast<Window *>(*state.root);
	window->shouldRedraw();
}

void Widget::reLayout() const {
	if (!*state.root) return;
	auto *window = reinterpret_cast<Window *>(*state.root);
	window->shouldRelayout();
}

void Widget::reArrange() const {
	if (!*state.root) return;
	auto *window = reinterpret_cast<Window *>(*state.root);
	window->shouldReposition();
}