#include "widget.hpp"

#include "functional"
#include "ranges"
#include "utils.hpp"
#include "window.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <print>


using namespace squi;

Widget::Widget(const Args &args, const FlagsArgs &flags)
	: flags(this, flags),
	  id(idCounter++),
	  state{
		  .width{this, args.width.value_or(Size::Expand)},
		  .height{this, args.height.value_or(Size::Expand)},
		  .sizeConstraints = args.sizeConstraints,
		  .margin{this, args.margin.value_or(Margin{})},
		  .padding{this, args.padding.value_or(Margin{})},
		  .parent{this, nullptr},
		  .root = Stateful<Widget *, StateImpact::RelayoutNeeded>{
			  [](Widget &parent, Widget *newRoot) {
				  bool initialized = parent.initialized;
				  parent.initialize();
				  for (auto &child: parent.getChildren()) {
					  child->state.root = newRoot;
				  }
				  for (auto &child: parent.childrenToAdd) {
					  child->state.root = newRoot;
				  }
				  if (!initialized) {
					  for (auto &func: parent.funcs().afterInit) {
						  if (func) func(parent);
					  }
					  parent.funcs().afterInit.clear();
				  }
			  },
			  this,
			  nullptr,
		  },
	  } {
	// if (args.customState) customState.add(args.customState.value());
	customState.add(args.customState);
	if (args.onInit) m_funcs.onInit.push_back(args.onInit);
	if (args.afterInit) m_funcs.afterInit.push_back(args.afterInit);
	if (args.onUpdate) m_funcs.onUpdate.push_back(args.onUpdate);
	if (args.afterUpdate) m_funcs.afterUpdate.push_back(args.afterUpdate);
	if (args.beforeLayout) m_funcs.beforeLayout.push_back(args.beforeLayout);
	if (args.onLayout) m_funcs.onLayout.push_back(args.onLayout);
	if (args.afterLayout) m_funcs.afterLayout.push_back(args.afterLayout);
	if (args.onArrange) m_funcs.onArrange.push_back(args.onArrange);
	if (args.afterArrange) m_funcs.onArrange.push_back(args.afterArrange);
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

	return {};
}

void Widget::setChildren(const Children &newChildren) {
	for (auto &child: children) {
		child->deleteLater();
	}
	for (auto &child: childrenToAdd) {
		child->deleteLater();
	}
	childrenToAdd.reserve(childrenToAdd.size() + newChildren.size());
	for (const auto &child: newChildren) {
		if (child) {
			child->state.parent = this;
			child->state.root = state.root;
			if (!child->initialized) child->initialize();
			childrenToAdd.emplace_back(child);
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
		child->state.root = state.root;
		if (!child->initialized) child->initialize();
		childrenToAdd.push_back(child);
		for (auto &func: m_funcs.onChildAdded) {
			if (func) func(*this, child);
		}
		reLayout();
	}
}

void Widget::updateChildren() {
	for (auto &child: std::views::reverse(children)) {
		child->update();
	}
}

void Widget::insertChildren() {
	// Add the children to the list
	// These don't get to be updated this frame because of the inconsistency they would bring
	if (!childrenToAdd.empty()) {
		children.insert(children.end(), childrenToAdd.begin(), childrenToAdd.end());
		childrenToAdd.clear();
		reLayout();
	}
}

void Widget::update() {
#ifndef NDEBUG
	for (auto &func: m_funcs.onDebugUpdate) {
		if (func) func();
	}
#endif

	if (shouldDelete) return;

	insertChildren();

	// On update
	for (auto &func: m_funcs.onUpdate) {
		if (func) func(*this);
	}
	onUpdate();

	// Update the children
	if (flags.shouldUpdateChildren && *flags.visible) {
		updateChildren();
	}
	auto [eraseFirst, eraseLast] = std::ranges::remove_if(children, [&](const Child &child) -> bool {
		if (child->shouldDelete) {
			for (auto &func: m_funcs.onChildRemoved) {
				if (func) func(*this, child);
			}
			reLayout();
		}
		return child->shouldDelete;
	});
	children.erase(eraseFirst, eraseLast);

	insertChildren();

	// After update
	for (auto &func: m_funcs.afterUpdate) {
		if (func) func(*this);
	}
	afterUpdate();
}

vec2 Widget::layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) {
	vec2 contentSize{};

	for (auto &child: children) {
		const auto size = child->layout(maxSize, minSize, shouldShrink, final);
		contentSize.x = std::max(size.x, contentSize.x);
		contentSize.y = std::max(size.y, contentSize.y);
	}

	return contentSize;
}

vec2 squi::Widget::layout(vec2 maxSize, vec2 minSize, ShouldShrink forceShrink, bool final) {
#ifndef NDEBUG
	for (auto &func: m_funcs.onDebugLayout) {
		if (func) func();
	}
#endif

	if (shouldDelete) return {};

	// Make sure all the children are added to the list before layout
	insertChildren();

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
	std::visit(
		utils::overloaded{
			[&](const float &val) {
				minSize.x = padding.x;
				maxSize.x = std::clamp(val, minSize.x, maxSize.x);
				shouldShrink.width = false;
			},
			[&](const Size &size) {
				if (size == Size::Shrink) {
					shouldShrink.width = true;
					minSize.x = padding.x;
				} else {
					minSize.x = std::clamp(minSize.x - margin.x, padding.x, maxSize.x);
				}
			},
		},
		*state.width
	);

	std::visit(
		utils::overloaded{
			[&](const float &val) {
				minSize.y = padding.y;
				maxSize.y = std::clamp(val, minSize.y, maxSize.y);
				shouldShrink.height = false;
			},
			[&](const Size &size) {
				if (size == Size::Shrink) {
					shouldShrink.height = true;
					minSize.y = padding.y;
				} else {
					minSize.y = std::clamp(minSize.y - margin.y, padding.y, maxSize.y);
				}
			},
		},
		*state.height
	);

	// Handle the min size constraints
	if (constraints.minWidth.has_value()) {
		minSize.x = std::clamp(*constraints.minWidth, minSize.x, maxSize.x);
	}
	if (constraints.minHeight.has_value()) {
		minSize.y = std::clamp(*constraints.minHeight, minSize.y, maxSize.y);
	}

#ifndef NDEBUG
	if (state.root) {
		auto &window = Window::of(this);
		window.relayoutCounter[id]++;
	}
#endif

	for (auto &func: m_funcs.onLayout) {
		if (func) func(*this, maxSize, minSize);
	}
	onLayout(maxSize, minSize);

	if (flags.shouldLayoutChildren) {
		const auto contentSize = layoutChildren(maxSize - padding, minSize - padding, shouldShrink, final);
		minSize.x = std::clamp(contentSize.x + padding.x, minSize.x, maxSize.x);
		minSize.y = std::clamp(contentSize.y + padding.y, minSize.y, maxSize.y);
	}

	std::visit(
		utils::overloaded{
			[&](const float &val) {
				size.x = std::clamp(val, minSize.x, maxSize.x);
			},
			[&](const Size &val) {
				switch (val) {
					case Size::Expand: {
						if (shouldShrink.width)
							size.x = minSize.x;
						else
							size.x = maxSize.x;
						break;
					}
					case Size::Wrap:
					case Size::Shrink: {
						size.x = minSize.x;
						break;
					}
				}
			},
		},
		*state.width
	);

	std::visit(
		utils::overloaded{
			[&](const float &val) {
				size.y = std::clamp(val, minSize.y, maxSize.y);
			},
			[&](const Size &val) {
				switch (val) {
					case Size::Expand: {
						if (shouldShrink.height)
							size.y = minSize.y;
						else
							size.y = maxSize.y;
						break;
					}
					case Size::Wrap:
					case Size::Shrink: {
						size.y = minSize.y;
						break;
					}
				}
			},
		},
		*state.height
	);

	if (final) {
		for (auto &func: m_funcs.afterLayout) {
			if (func) func(*this, size);
		}
		postLayout(size);
	}

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

	if (shouldDelete) return;

	if (!*flags.visible) return;
	for (auto &onArrange: m_funcs.onArrange) {
		if (onArrange) onArrange(*this, pos);
	}
	onArrange(pos);
	this->pos = pos;

	if (flags.shouldArrangeChildren) {
		arrangeChildren(pos);
	}
	for (auto &afterArrange: m_funcs.afterArrange) {
		if (afterArrange) afterArrange(*this, pos);
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
	if (initialized) return;
	if (!*state.root || !*state.parent) return;
	initialized = true;
	for (auto &func: m_funcs.onInit) {
		if (func) func(*this);
	}
	m_funcs.onInit.clear();
}

void Widget::reDraw() const {
	if (*state.root == nullptr) return;
	auto *window = reinterpret_cast<Window *>(*state.root);
	window->shouldRedraw();
}

void Widget::reLayout() const {
	if (*state.root == nullptr) return;
	auto *window = reinterpret_cast<Window *>(*state.root);
	window->shouldRelayout();
}

void Widget::reArrange() const {
	if (*state.root == nullptr) return;
	auto *window = reinterpret_cast<Window *>(*state.root);
	window->shouldReposition();
}