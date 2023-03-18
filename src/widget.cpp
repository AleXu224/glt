#include "widget.hpp"

using namespace squi;

Widget::Widget(const Args& args, const Options &options)
	: isContainer(options.isContainer),
	  shouldUpdateChildren(options.shouldUpdateChildren),
	  shouldDrawChildren(options.shouldDrawChildren),
	  shouldHandleSizeBehavior(options.shouldHandleSizeBehavior),
	  isInteractive(options.isInteractive),
	  onInitArg(args.onInit),
	  beforeUpdateArg(args.beforeUpdate),
	  onUpdateArg(args.onUpdate),
	  afterUpdateArg(args.afterUpdate),
	  m_data(Widget::Data{
		  .size = args.size,
		  .margin = args.margin,
		  .padding = args.padding,
		  .sizeBehavior = args.sizeBehavior,
	  }) {
	// TODO: Add these on the Child class that should act as a factory
	//	init();
	//	if (args.onInit) {
	//		args.onInit(*this);
	//	}
}

Widget::Data &Widget::data() {
	if (isContainer && !children.empty()) {
		return children.front()->data();
	} else {
		return m_data;
	}
}

const Widget::Data &Widget::data() const {
	if (isContainer && !children.empty()) {
		return children.front()->data();
	} else {
		return m_data;
	}
}

const std::vector<std::shared_ptr<Widget>> &Widget::getChildren() const {
	if (isContainer && !children.empty())
		return children.front()->getChildren();
	else
		return children;
}

Rect Widget::getRect() const {
	const auto& data = this->data();
	return Rect::fromPosSize(data.pos + data.margin.getPositionOffset(), data.size);
}

Rect Widget::getContentRect() const {
	const auto& data = this->data();
	return Rect::fromPosSize(
		data.pos + data.margin.getPositionOffset() + data.padding.getPositionOffset(),
		data.size - data.padding.getSizeOffset());
}

Rect Widget::getLayoutRect() const {
	const auto& data = this->data();
	return Rect::fromPosSize(
		data.pos,
		data.size + data.padding.getSizeOffset());
}

std::vector<Rect> Widget::getHitcheckRect() const {
	if (isInteractive)
		return {getRect()};
	else
		return {};
}

void Widget::setChildren(const std::vector<std::shared_ptr<Widget>> &newChildren) {
	children = newChildren;
}

void Widget::addChild(const Child &child) {
	if (child.hasChild())
		children.push_back(child);
}

void Widget::update() {
	if (beforeUpdateArg) beforeUpdateArg(*this);

	// Check if the size hint is set
	bool horizontalHint = m_data.sizeHint.x != -1;
	bool verticalHint = m_data.sizeHint.y != -1;

	// Expand the Widget
	// Should be done before the child is updated so that the child can get accurate size data
	if (!isContainer && shouldHandleSizeBehavior && m_data.parent != nullptr) {
		if (!horizontalHint && m_data.sizeBehavior.horizontal == SizeBehaviorType::FillParent)
			m_data.size.x = m_data.parent->getContentRect().width() - m_data.margin.getSizeOffset().x;
		if (!verticalHint && m_data.sizeBehavior.vertical == SizeBehaviorType::FillParent)
			m_data.size.y = m_data.parent->getContentRect().height() - m_data.margin.getSizeOffset().y;
	}

	// On update
	if (onUpdateArg) onUpdateArg(*this);
	onUpdate();

	// Update the children
	if (shouldUpdateChildren) {
		const auto childPos = m_data.pos + m_data.margin.getPositionOffset() + m_data.padding.getPositionOffset();
		for (auto &child: children) {
			child->m_data.parent = this;
			child->m_data.pos = childPos;
			child->update();
		}
	}

	afterChildrenUpdate();

	// Shrink the Widget
	// Should be done after the child is updated since it depends on the child's size
	if (!isContainer && shouldHandleSizeBehavior && !children.empty()) {
		const auto &child = children.front();
		if (!horizontalHint && m_data.sizeBehavior.horizontal == SizeBehaviorType::MatchChild)
			m_data.size.x = child->getLayoutRect().width() + m_data.padding.getSizeOffset().x;
		if (!verticalHint && m_data.sizeBehavior.vertical == SizeBehaviorType::MatchChild)
			m_data.size.y = child->getLayoutRect().height() + m_data.padding.getSizeOffset().y;
	}

	// After update
	afterUpdate();
	if (afterUpdateArg) afterUpdateArg(*this);
}

void Widget::draw() {
	onDraw();

	if (shouldDrawChildren) {
		for (auto &child: children) {
			child->draw();
		}
	}
}

void Widget::initialize() {
	if (isInitialized) return;
	isInitialized = true;
	init();
	if (onInitArg) onInitArg(*this);
}
