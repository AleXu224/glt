#include "widget.hpp"

using namespace squi;

Widget::Widget(const Args &args, const Options &options)
	: data(args),
	  isContainer(options.isContainer),
	  shouldUpdateChildren(options.shouldUpdateChildren) {
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

void Widget::addChild(const std::shared_ptr<Widget>& child) {
	children.push_back(child);
}

void Widget::update() {
	if (data.beforeUpdate) data.beforeUpdate(*this);

	// Expand the Widget
	// Should be done before the child is update so that the child can get accurate size data
//	if (data.sizeBehavior.horizontal == SizeBehaviorType::FillParent)

	// TODO: add getContentSize, getSize, getLayoutSize
}
