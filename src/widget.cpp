#include "widget.hpp"
#include "ranges"
#include <optional>

using namespace squi;

Widget::Widget(const Args& args, const Options &options)
	: shouldUpdateChildren(options.shouldUpdateChildren),
	  shouldDrawChildren(options.shouldDrawChildren),
	  shouldHandleSizeBehavior(options.shouldHandleSizeBehavior),
	  m_funcs{
		.onInit = args.onInit,
		.beforeUpdate = args.beforeUpdate,
		.onUpdate = args.onUpdate,
		.afterUpdate = args.afterUpdate,
		.beforeDraw = args.beforeDraw,
		.onDraw = args.onDraw,
		.afterDraw = args.afterDraw,
	  },
	  m_data(Widget::Data{
		  .size = args.size,
		  .margin = args.margin,
		  .padding = args.padding,
		  .sizeBehavior = args.sizeBehavior,
		  .gestureDetector = GestureDetector(this),
		  .isInteractive = options.isInteractive,
	  }) {
	// TODO: Add these on the Child class that should act as a factory
	//	init();
	//	if (args.onInit) {
	//		args.onInit(*this);
	//	}
}

Widget::Data &Widget::data() {
	return m_data;
}

const Widget::Data &Widget::data() const {
	return m_data;
}

Widget::FunctionArgs &Widget::funcs() {
	return m_funcs;
}

const Widget::FunctionArgs &Widget::funcs() const {
	return m_funcs;
}

const std::vector<std::shared_ptr<Widget>> &Widget::getChildren() const {
	// if (isContainer && !children.empty())
	// 	return children.front()->getChildren();
	// else
		return children;
}

// Rect Widget::getRect() const {
// 	const auto& data = this->data();
// 	return Rect::fromPosSize(data.pos + data.margin.getPositionOffset(), data.size);
// }

// Rect Widget::getContentRect() const {
// 	const auto& data = this->data();
// 	return Rect::fromPosSize(
// 		data.pos + data.margin.getPositionOffset() + data.padding.getPositionOffset(),
// 		data.size - data.padding.getSizeOffset());
// }

// Rect Widget::getLayoutRect() const {
// 	const auto& data = this->data();
// 	return Rect::fromPosSize(
// 		data.pos,
// 		data.size + data.margin.getSizeOffset());
// }

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

void Widget::fillParentSizeBehavior(bool horizontalHint, bool verticalHint) {
	if (!horizontalHint && m_data.sizeBehavior.horizontal == SizeBehaviorType::FillParent)
		m_data.size.x = m_data.parent->getContentRect().width() - m_data.margin.getSizeOffset().x;
	if (!verticalHint && m_data.sizeBehavior.vertical == SizeBehaviorType::FillParent)
		m_data.size.y = m_data.parent->getContentRect().height() - m_data.margin.getSizeOffset().y;
}

void Widget::matchChildSizeBehavior(bool horizontalHint, bool verticalHint) {
	const auto &child = children.front();
	if (!horizontalHint && m_data.sizeBehavior.horizontal == SizeBehaviorType::MatchChild)
		m_data.size.x = child->getLayoutRect().width() + m_data.padding.getSizeOffset().x;
	if (!verticalHint && m_data.sizeBehavior.vertical == SizeBehaviorType::MatchChild)
		m_data.size.y = child->getLayoutRect().height() + m_data.padding.getSizeOffset().y;
}

void Widget::update() {
	if (m_funcs.beforeUpdate) m_funcs.beforeUpdate(*this);

	// Update the GestureDetector
	// TODO: This will be executed even if the Widget does not need the input
	// 	     Should somehow check if the Widget will need the input in the update
	m_data.gestureDetector.update();

	// Size hint
	bool horizontalHint = m_data.sizeHint.x != -1;
	bool verticalHint = m_data.sizeHint.y != -1;
	if (horizontalHint) m_data.size.x = m_data.sizeHint.x;
	if (verticalHint) m_data.size.y = m_data.sizeHint.y;

	// Expand the Widget
	// Should be done before the child is updated so that the child can get accurate size data
	if (shouldHandleSizeBehavior && m_data.parent != nullptr) {
		fillParentSizeBehavior(horizontalHint, verticalHint);
	}

	// On update
	if (m_funcs.onUpdate) m_funcs.onUpdate(*this);
	onUpdate();

	// Update the children
	if (shouldUpdateChildren) {
		for (auto &child: std::views::reverse(children)) {
			child->m_data.parent = this;
			child->update();
		}
	}

	afterChildrenUpdate();

	// Shrink the Widget
	// Should be done after the child is updated since it depends on the child's size
	if (shouldHandleSizeBehavior && !children.empty()) {
		matchChildSizeBehavior(horizontalHint, verticalHint);
	}

	// After update
	afterUpdate();
	if (m_funcs.afterUpdate) m_funcs.afterUpdate(*this);
}

void Widget::draw() {
	if (m_funcs.beforeDraw) m_funcs.beforeDraw(*this);
	if (!m_data.visible) return;
	if (m_funcs.onDraw) m_funcs.onDraw(*this);
	onDraw();

	if (shouldDrawChildren) {
		const auto childPos = m_data.pos + m_data.margin.getPositionOffset() + m_data.padding.getPositionOffset();
		for (auto &child: children) {
			child->m_data.pos = childPos;
			child->draw();
		}
	}

	if (m_funcs.afterDraw) m_funcs.afterDraw(*this);
}

void Widget::initialize() {
	if (isInitialized) return;
	isInitialized = true;
	init();
	if (m_funcs.onInit) m_funcs.onInit(*this);
}
