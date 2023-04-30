#include "widget.hpp"
#include "ranges"
#include "utility"
#include <algorithm>
#include <numeric>
#include <optional>


using namespace squi;

uint32_t Widget::widgetCount = 0;

Widget::Widget(const Args &args, const Options &options)
	: shouldUpdateChildren(options.shouldUpdateChildren),
	  shouldDrawChildren(options.shouldDrawChildren),
	  shouldHandleLayout(options.shouldHandleLayout),
	  shouldArrangeChildren(options.shouldArrangeChildren),
	  m_data(Widget::Data{
		  .sizeMode{
			  .width = args.width,
			  .height = args.height,
		  },
		  .sizeConstraints = args.sizeConstraints,
		  .margin = args.margin,
		  .padding = args.padding,
		  .isInteractive = options.isInteractive,
	  }) {
	if (args.onInit) m_funcs.onInit.push_back(args.onInit);
	if (args.onUpdate) m_funcs.onUpdate.push_back(args.onUpdate);
	if (args.afterUpdate) m_funcs.afterUpdate.push_back(args.afterUpdate);
	if (args.onLayout) m_funcs.onLayout.push_back(args.onLayout);
	if (args.onArrange) m_funcs.onArrange.push_back(args.onArrange);
	if (args.beforeDraw) m_funcs.beforeDraw.push_back(args.beforeDraw);
	if (args.onDraw) m_funcs.onDraw.push_back(args.onDraw);
	if (args.afterDraw) m_funcs.afterDraw.push_back(args.afterDraw);
	widgetCount++;
}

Widget::~Widget() {
	widgetCount--;
}

Widget::FunctionArgs &Widget::funcs() {
	return m_funcs;
}

const Widget::FunctionArgs &Widget::funcs() const {
	return m_funcs;
}

std::vector<std::shared_ptr<Widget>> &Widget::getChildren() {
	return children;
}

std::optional<Rect> Widget::getHitcheckRect() const {
	if (m_data.isInteractive)
		return getRect();
	else
		return std::nullopt;
}

void Widget::setChildren(const std::vector<std::shared_ptr<Widget>> &newChildren) {
	children = newChildren;
}

void Widget::addChild(const Child &child) {
	if (child.hasChild())
		children.push_back(child);
}

void Widget::update() {
	// On update
	for (auto &func: m_funcs.onUpdate) {
		if (func) func(*this);
	}
	onUpdate();

	// Update the children
	if (shouldUpdateChildren) {
		for (auto &child: std::views::reverse(children)) {
			child->m_data.parent = this;
			child->update();
		}
	}
	children.erase(std::remove_if(children.begin(), children.end(), [](const auto &child) {
					   return child->m_data.shouldDelete;
				   }),
				   children.end());

	// After update
	for (auto &func: m_funcs.afterUpdate) {
		if (func) func(*this);
	}
	afterUpdate();
}

vec2 squi::Widget::layout(vec2 maxSize) {
	const auto &constraints = m_data.sizeConstraints;

	maxSize -= m_data.margin.getSizeOffset();

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

	for (auto &func: m_funcs.onLayout) {
		if (func) func(*this, maxSize, minSize);
	}
	onLayout(maxSize, minSize);
	if (m_data.sizeMode.width.index() == 1 && std::get<1>(m_data.sizeMode.width) == Size::Shrink) {
		maxSize.x = std::min(getMinWidth() - m_data.margin.getSizeOffset().x, maxSize.x);
	}
	if (m_data.sizeMode.height.index() == 1 && std::get<1>(m_data.sizeMode.height) == Size::Shrink) {
		maxSize.y = std::min(getMinHeight() - m_data.margin.getSizeOffset().y, maxSize.y);
	}
	if (shouldHandleLayout) {
		vec2 childMaxSize = maxSize - m_data.padding.getSizeOffset();
		if (m_data.sizeMode.width.index() == 0) {
			childMaxSize.x = std::min(childMaxSize.x, std::get<0>(m_data.sizeMode.width));
		}
		if (m_data.sizeMode.height.index() == 0) {
			childMaxSize.y = std::min(childMaxSize.y, std::get<0>(m_data.sizeMode.height));
		}

		for (auto &child: children) {
			const auto size = child->layout(childMaxSize);
			minSize.x = std::clamp(size.x, minSize.x, maxSize.x);
			minSize.y = std::clamp(size.y, minSize.y, maxSize.y);
		}

		minSize += m_data.padding.getSizeOffset();
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

	switch (m_data.sizeMode.width.index()) {
		case 0: {
			size.x = std::clamp(std::get<0>(m_data.sizeMode.width), minSize.x, maxSize.x);
			break;
		}
		case 1: {
			switch (std::get<1>(m_data.sizeMode.width)) {
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
	switch (m_data.sizeMode.height.index()) {
		case 0: {
			size.y = std::clamp(std::get<0>(m_data.sizeMode.height), minSize.y, maxSize.y);
			break;
		}
		case 1: {
			switch (std::get<1>(m_data.sizeMode.height)) {
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

	return size + m_data.margin.getSizeOffset();
}

float squi::Widget::getMinWidth() {
	switch (m_data.sizeMode.width.index()) {
		case 0: {
			return std::get<0>(m_data.sizeMode.width) + m_data.margin.getSizeOffset().x;
		}
		case 1: {
			return m_data.margin.getSizeOffset().x + m_data.padding.getSizeOffset().x + std::accumulate(children.begin(), children.end(), 0.0f, [](float acc, const auto &child) {
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
	switch (m_data.sizeMode.height.index()) {
		case 0: {
			return std::get<0>(m_data.sizeMode.height) + m_data.margin.getSizeOffset().y;
		}
		case 1: {
			return m_data.margin.getSizeOffset().y + m_data.padding.getSizeOffset().y + std::accumulate(children.begin(), children.end(), 0.0f, [](float acc, const auto &child) {
					   return std::max(acc, child->getMinHeight());
				   });
		}
		default: {
			std::unreachable();
			return 0.0f;
		}
	}
}

void Widget::arrange(vec2 pos) {
	if (!m_data.visible) return;
	for (auto &onArrange: m_funcs.onArrange) {
		if (onArrange) onArrange(*this, pos);
	}
	onArrange(pos);
	this->pos = pos;

	if (shouldArrangeChildren) {
		const auto childPos = pos + m_data.margin.getPositionOffset() + m_data.padding.getPositionOffset();
		for (auto &child: children) {
			child->arrange(childPos);
		}
	}
	postArrange(pos);
}

void Widget::draw() {
	for (auto &func: m_funcs.beforeDraw) {
		if (func) func(*this);
	}
	if (!m_data.visible) return;
	for (auto &func: m_funcs.onDraw) {
		if (func) func(*this);
	}
	onDraw();

	if (shouldDrawChildren) {
		for (auto &child: children) {
			child->draw();
		}
	}

	for (auto &func: m_funcs.afterDraw) {
		if (func) func(*this);
	}
}

void Widget::initialize() {
	if (isInitialized) return;
	isInitialized = true;
	init();
	for (auto &func: m_funcs.onInit) {
		if (func) func(*this);
	}
}