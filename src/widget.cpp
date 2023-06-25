#include <list>
#include <memory>
#include <unordered_map>
#define NOMINMAX
#include "quad.hpp"
#include "ranges"
#include "renderer.hpp"
#include "utility"
#include "widget.hpp"
#include <algorithm>
#include <numeric>
#include <optional>



using namespace squi;

// std::unordered_map<uint64_t, Widget *> Widget::Store::widgets{
// 	{0, nullptr},
// };
uint64_t Widget::idCounter = 1;// 0 is reserved for exceptions
uint32_t Widget::widgetCount = 0;

Widget::Widget(const Args &args, const Flags &flags)
	: flags(flags),
	  state(Widget::State{
		  .sizeMode{
			  .width = args.width,
			  .height = args.height,
		  },
		  .sizeConstraints = args.sizeConstraints,
		  .margin = args.margin,
		  .padding = args.padding,
	  }),
	  id(idCounter++) {
	if (args.onInit) m_funcs.onInit.push_back(args.onInit);
	if (args.onUpdate) m_funcs.onUpdate.push_back(args.onUpdate);
	if (args.afterUpdate) m_funcs.afterUpdate.push_back(args.afterUpdate);
	if (args.onLayout) m_funcs.onLayout.push_back(args.onLayout);
	if (args.onArrange) m_funcs.onArrange.push_back(args.onArrange);
	if (args.beforeDraw) m_funcs.beforeDraw.push_back(args.beforeDraw);
	if (args.onDraw) m_funcs.onDraw.push_back(args.onDraw);
	if (args.afterDraw) m_funcs.afterDraw.push_back(args.afterDraw);
	widgetCount++;
	initialize();
}

Widget::~Widget() {
	widgetCount--;

	// if (auto it = Store::widgets.find(id); it != Store::widgets.end()) {
	// 	Store::widgets.erase(it);
	// }
}

// Widget *Widget::Store::getWidget(uint64_t id) {
// 	if (auto it = widgets.find(id); it != widgets.end()) {
// 		return it->second;
// 	} else {
// 		return widgets[0];
// 	}
// }

Widget::FunctionArgs &Widget::funcs() {
	return m_funcs;
}

const Widget::FunctionArgs &Widget::funcs() const {
	return m_funcs;
}

std::vector<Child> &Widget::getChildren() {
	return children;
}

std::optional<Rect> Widget::getHitcheckRect() const {
	if (flags.isInteractive && flags.visible)
		return getRect();
	else
		return std::nullopt;
}

void Widget::setChildren(const Children &newChildren) {
	children = newChildren;
}

void Widget::addChild(const Child &child) {
	if (child)
		children.push_back(child);
}

void Widget::updateChildren() {
	for (auto &child: std::views::reverse(children)) {
		child->state.parent = this;
		child->update();
	}
}

void Widget::update() {
	// On update
	for (auto &func: m_funcs.onUpdate) {
		if (func) func(*this);
	}
	onUpdate();

	// Update the children
	if (flags.shouldUpdateChildren && flags.visible) {
		updateChildren();
	}
	children.erase(
		std::remove_if(children.begin(), children.end(), [](const Child &child) -> bool {
			return child->shouldDelete;
		}),
		children.end());

	// After update
	for (auto &func: m_funcs.afterUpdate) {
		if (func) func(*this);
	}
	afterUpdate();
}

void Widget::layoutChildren(vec2 &maxSize, vec2 &minSize) {
	vec2 childMaxSize = maxSize - state.padding.getSizeOffset();
	if (state.sizeMode.width.index() == 0) {
		childMaxSize.x = std::min(childMaxSize.x, std::get<0>(state.sizeMode.width));
	}
	if (state.sizeMode.height.index() == 0) {
		childMaxSize.y = std::min(childMaxSize.y, std::get<0>(state.sizeMode.height));
	}

	for (auto &child: children) {
		const auto size = child->layout(childMaxSize);
		minSize.x = std::clamp(size.x, minSize.x, maxSize.x);
		minSize.y = std::clamp(size.y, minSize.y, maxSize.y);
	}

	minSize += state.padding.getSizeOffset();
}

vec2 squi::Widget::layout(vec2 maxSize) {
	// An invisible widget has no size
	if (!flags.visible) return {0, 0};

	const auto &constraints = state.sizeConstraints;

	maxSize -= state.margin.getSizeOffset();

	// Handle the max size constraints
	if (constraints.maxWidth.has_value()) {
		maxSize.x = *constraints.maxWidth;
	}
	if (constraints.maxHeight.has_value()) {
		maxSize.y = *constraints.maxHeight;
	}

	vec2 minSize{
		std::min(0.0f, maxSize.x),
		std::min(0.0f, maxSize.y),
	};

	if (state.sizeMode.width.index() == 1 && std::get<1>(state.sizeMode.width) == Size::Shrink) {
		maxSize.x = std::min(getMinWidth() - state.margin.getSizeOffset().x, maxSize.x);
	}
	if (state.sizeMode.height.index() == 1 && std::get<1>(state.sizeMode.height) == Size::Shrink) {
		maxSize.y = std::min(getMinHeight() - state.margin.getSizeOffset().y, maxSize.y);
	}

	for (auto &func: m_funcs.onLayout) {
		if (func) func(*this, maxSize, minSize);
	}
	onLayout(maxSize, minSize);

	if (flags.shouldLayoutChildren) {
		layoutChildren(maxSize, minSize);
	}

	minSize.x = std::min(minSize.x, maxSize.x);
	minSize.y = std::min(minSize.y, maxSize.y);

	// Handle the min size constraints
	if (constraints.minWidth.has_value()) {
		minSize.x = std::max(minSize.x, *constraints.minWidth);
	}
	if (constraints.minHeight.has_value()) {
		minSize.y = std::max(minSize.y, *constraints.minHeight);
	}

	switch (state.sizeMode.width.index()) {
		case 0: {
			size.x = std::clamp(std::get<0>(state.sizeMode.width), minSize.x, maxSize.x);
			break;
		}
		case 1: {
			switch (std::get<1>(state.sizeMode.width)) {
				case Size::Expand: {
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
	switch (state.sizeMode.height.index()) {
		case 0: {
			size.y = std::clamp(std::get<0>(state.sizeMode.height), minSize.y, maxSize.y);
			break;
		}
		case 1: {
			switch (std::get<1>(state.sizeMode.height)) {
				case Size::Expand: {
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

	postLayout(size);

	return size + state.margin.getSizeOffset();
}

float squi::Widget::getMinWidth() {
	if (!flags.visible) return 0.0f;
	switch (state.sizeMode.width.index()) {
		case 0: {
			return std::get<0>(state.sizeMode.width) + state.margin.getSizeOffset().x;
		}
		case 1: {
			return state.margin.getSizeOffset().x + state.padding.getSizeOffset().x + std::accumulate(children.begin(), children.end(), 0.0f, [](float acc, const auto &child) {
					   return std::max(acc, child->getMinWidth());
				   });
		}
		default: {
			std::unreachable();
			return 0.0f;
		}
	}
}

float squi::Widget::getMinHeight() {
	if (!flags.visible) return 0.0f;
	switch (state.sizeMode.height.index()) {
		case 0: {
			return std::get<0>(state.sizeMode.height) + state.margin.getSizeOffset().y;
		}
		case 1: {
			return state.margin.getSizeOffset().y + state.padding.getSizeOffset().y + std::accumulate(children.begin(), children.end(), 0.0f, [](float acc, const auto &child) {
					   return std::max(acc, child->getMinHeight());
				   });
		}
		default: {
			std::unreachable();
			return 0.0f;
		}
	}
}

void Widget::arrangeChildren(vec2 &pos) {
	const auto childPos = pos + state.margin.getPositionOffset() + state.padding.getPositionOffset();
	for (auto &child: children) {
		child->arrange(childPos);
	}
}

void Widget::arrange(vec2 pos) {
	if (!flags.visible) return;
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
	for (auto &func: m_funcs.beforeDraw) {
		if (func) func(*this);
	}
	if (!flags.visible) return;
	for (auto &func: m_funcs.onDraw) {
		if (func) func(*this);
	}
	onDraw();

	if (flags.shouldDrawChildren) {
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