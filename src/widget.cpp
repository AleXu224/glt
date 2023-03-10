#include "widget.hpp"

using namespace squi;

Widget::Widget(const Args &args, const Options &options)
	: data(args),
	  isContainer(options.isContainer),
	  shouldUpdateChildren(options.shouldUpdateChildren),
	  shouldDrawChildren(options.shouldDrawChildren),
	  shouldHandleSizeBehavior(options.shouldHandleSizeBehavior),
	  isInteractive(options.isInteractive) {
	init();
	if (args.onInit) {
		args.onInit(*this);
	}
}

const Widget::Args &Widget::getData() const {
	if (isContainer && !children.empty())
		return children.front()->getData();
	else
		return data;
}

Widget::Args &Widget::getData() {
	if (isContainer && !children.empty())
		return children.front()->getData();
	else
		return data;
}

const vec2 &Widget::getSize() const {
	return getData().size;
}

const vec2 &Widget::getPos() const {
	if (isContainer && !children.empty())
		return children.front()->getPos();
	else
		return pos;
}

const vec2 &Widget::getSizeHint() const {
	if (isContainer && !children.empty())
		return children.front()->getSizeHint();
	else
		return sizeHint;
}

const Margin &Widget::getMargin() const {
	return getData().margin;
}

const Margin &Widget::getPadding() const {
	return getData().padding;
}

const SizeBehavior &Widget::getSizeBehavior() const {
	return getData().sizeBehavior;
}

Widget &Widget::getParent() const {
	return *parent;
}

const std::vector<std::shared_ptr<Widget>> &Widget::getChildren() const {
	if (isContainer && !children.empty())
		return children.front()->getChildren();
	else
		return children;
}

Rect Widget::getRect() const {
	return Rect::fromPosSize(getPos() + getMargin().getPositionOffset(), getSize());
}

Rect Widget::getContentRect() const {
	const auto &margin = getMargin();
	const auto &padding = getPadding();
	return Rect::fromPosSize(
		getPos() + margin.getPositionOffset() + padding.getPositionOffset(),
		getSize() - padding.getSizeOffset());
}

Rect Widget::getLayoutRect() const {
	return Rect::fromPosSize(
		getPos(),
		getSize() + getPadding().getSizeOffset());
}

std::vector<Rect> Widget::getHitcheckRect() const {
	if (isInteractive)
		return {getRect()};
	else
		return {};
}

void Widget::setSize(const vec2 &newSize) {
	getData().size = newSize;
}

void Widget::setPos(const vec2 &newPos) {
	pos = newPos;
}

void Widget::setSizeHint(const vec2 &newSizeHint) {
	if (isContainer && !children.empty())
		children.front()->setSizeHint(newSizeHint);
	else
		sizeHint = newSizeHint;
}

void Widget::setMargin(const Margin &newMargin) {
	getData().margin = newMargin;
}

void Widget::setPadding(const Margin &newPadding) {
	getData().padding = newPadding;
}

void Widget::setSizeBehavior(const SizeBehavior &newSizeBehavior) {
	getData().sizeBehavior = newSizeBehavior;
}

void Widget::setParent(Widget *newParent) {
	parent = newParent;
}

void Widget::setChildren(const std::vector<std::shared_ptr<Widget>> &newChildren) {
	children = newChildren;
}

void Widget::addChild(const std::shared_ptr<Widget> &child) {
	children.push_back(child);
}

void Widget::update() {
	if (data.beforeUpdate) data.beforeUpdate(*this);

	// Check if the size hint is set
	bool horizontalHint = sizeHint.x != -1;
	bool verticalHint = sizeHint.y != -1;

	// Expand the Widget
	// Should be done before the child is updated so that the child can get accurate size data
	if (shouldHandleSizeBehavior && parent != nullptr) {
		if (!horizontalHint && data.sizeBehavior.horizontal == SizeBehaviorType::FillParent)
			data.size.x = parent->getContentRect().width() - data.margin.getSizeOffset().x;
		if (!verticalHint && data.sizeBehavior.vertical == SizeBehaviorType::FillParent)
			data.size.y = parent->getContentRect().height() - data.margin.getSizeOffset().y;
	}

	// On update
	if (data.onUpdate) data.onUpdate(*this);
	onUpdate();

	// Update the children
	if (shouldUpdateChildren) {
		const auto childPos = getPos() + getMargin().getPositionOffset() + getPadding().getPositionOffset();
		for (auto &child : children) {
			child->setParent(this);
			child->setPos(childPos);
			child->update();
		}
	}

	afterChildrenUpdate();

	// Shrink the Widget
	// Should be done after the child is updated since it depends on the child's size
	if (shouldHandleSizeBehavior && !children.empty()) {
		const auto &child = children.front();
		if (!horizontalHint && data.sizeBehavior.horizontal == SizeBehaviorType::MatchChild)
			data.size.x = child->getLayoutRect().width() + data.padding.getSizeOffset().x;
		if (!verticalHint && data.sizeBehavior.vertical == SizeBehaviorType::MatchChild)
			data.size.y = child->getLayoutRect().height() + data.padding.getSizeOffset().y;
	}

	// After update
	if (data.afterUpdate) data.afterUpdate(*this);
}

void Widget::draw() {
	onDraw();

	if (shouldDrawChildren) {
		for (auto &child : children) {
			child->draw();
		}
	}
}
