#include "element.hpp"

#include "core/app.hpp"
#include "widget.hpp"
#include <map>

namespace squi::core {
	// Element
	App *Element::getApp() const {
		assert(this->root != nullptr);
		auto *root = dynamic_cast<RootWidget::Element *>(this->root);
		assert(root);
		return root->app;
	}

	void Element::rebuild() {
		assert(this->mounted);
		if (this->dirty) {
			this->dirty = false;
			getApp()->dirtyElements.erase(this);
		}
	}

	void Element::markNeedsRebuild() {
		this->dirty = true;
		getApp()->dirtyElements.insert(this);
	}

	void Element::markNeedsRelayout() const {
		getApp()->needsRelayout = true;
	}

	void Element::markNeedsReposition() const {
		getApp()->needsReposition = true;
	}

	void Element::markNeedsRedraw() const {
		getApp()->needsRedraw = true;
	}

	void Element::addPostLayoutTask(const std::function<void()> &task) const {
		getApp()->postLayoutTasks.emplace_back(task);
	}

	ElementPtr Element::updateChild(ElementPtr child, const WidgetPtr &newWidget, size_t index, size_t depth) {
		if (!newWidget) {
			if (child) {
				child->unmount();
			}
			return nullptr;
		}

		if (child && child->widget == newWidget) {
			if (child->index != index) child->updateIndex(index);
			// No change
			return child;
		}

		if (child && child->widget && Widget::canUpdate(child->widget, newWidget)) {
			// Same widget type, update in place
			if (child->index != index) child->updateIndex(index);
			child->update(newWidget);
			return child;
		}
		// Different widget type or null, replace
		if (child) {
			child->unmount();
		}
		auto newChild = newWidget->_createElement();
		newChild->mount(this, index, depth);
		return newChild;
	}

	void Element::updateIndex(size_t index) {
		this->index = index;
	}

	// Implementation is straight up copied from Flutter
	void Element::updateChildren(std::vector<ElementPtr> &oldChildren, const std::vector<WidgetPtr> &newWidgets) {
		int64_t newChildrenTop = 0;
		int64_t oldChildrenTop = 0;
		int64_t newChildrenBottom = static_cast<int64_t>(newWidgets.size()) - 1;
		int64_t oldChildrenBottom = static_cast<int64_t>(oldChildren.size()) - 1;

		std::vector<ElementPtr> newChildren(newWidgets.size(), nullptr);

		while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
			auto &oldChild = oldChildren.at(oldChildrenTop);
			const auto &newWidget = newWidgets.at(newChildrenTop);

			if (!Widget::canUpdate(oldChild->widget, newWidget)) {
				break;
			}

			auto newChild = updateChild(oldChild, newWidget, newChildrenTop, this->depth + 1);

			newChildren.at(newChildrenTop) = newChild;
			newChildrenTop++;
			oldChildrenTop++;
		}

		while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
			auto &oldChild = oldChildren.at(oldChildrenBottom);
			const auto &newWidget = newWidgets.at(newChildrenBottom);

			if (!Widget::canUpdate(oldChild->widget, newWidget)) {
				break;
			}

			oldChildrenBottom--;
			newChildrenBottom--;
		}

		std::vector<ElementPtr> childrenToRemove{};

		bool haveOldChildren = oldChildrenTop <= oldChildrenBottom;
		std::map<size_t, ElementPtr> oldKeyedChildren{};
		if (haveOldChildren) {
			while (oldChildrenTop <= oldChildrenBottom) {
				auto &oldChild = oldChildren.at(oldChildrenTop);
				if (oldChild->widget->getKey() != nullptr) {
					oldKeyedChildren[oldChild->widget->getKey().hash()] = oldChild;
				} else {
					childrenToRemove.push_back(oldChild);
				}
				oldChildrenTop++;
			}
		}

		while (newChildrenTop <= newChildrenBottom) {
			ElementPtr oldChild = nullptr;
			const auto &newWidget = newWidgets.at(newChildrenTop);
			if (haveOldChildren) {
				const auto &key = newWidget->getKey();
				if (key != nullptr && oldKeyedChildren.contains(key.hash())) {
					oldChild = oldKeyedChildren[key.hash()];
					if (Widget::canUpdate(oldChild->widget, newWidget)) {
						oldKeyedChildren.erase(key.hash());
					} else {
						oldChild.reset();
					}
				}
			}
			auto newChild = updateChild(oldChild, newWidget, newChildrenTop, this->depth + 1);
			newChildren.at(newChildrenTop) = newChild;
			newChildrenTop++;
		}

		newChildrenBottom = static_cast<int64_t>(newWidgets.size()) - 1;
		oldChildrenBottom = static_cast<int64_t>(oldChildren.size()) - 1;

		while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
			auto &oldChild = oldChildren.at(oldChildrenTop);
			const auto &newWidget = newWidgets.at(newChildrenTop);

			auto newChild = updateChild(oldChild, newWidget, newChildrenTop, this->depth + 1);

			newChildren.at(newChildrenTop) = newChild;
			newChildrenTop++;
			oldChildrenTop++;
		}

		if (haveOldChildren && !oldKeyedChildren.empty()) {
			for (const auto &[key, oldChild]: oldKeyedChildren) {
				childrenToRemove.push_back(oldChild);
			}
		}

		for (const auto &child: childrenToRemove) {
			child->unmount();
		}

		oldChildren = std::move(newChildren);
	}

	// Component Element
	void ComponentElement::firstBuild() {
		auto childWidget = build();
		if (childWidget) {
			this->child = childWidget->_createElement();
			this->child->mount(this, this->index, this->depth + 1);
		}
	}

	void ComponentElement::mount(Element *parent, size_t index, size_t depth) {
		Element::mount(parent, index, depth);

		this->firstBuild();
	}

	void ComponentElement::rebuild() {
		assert(this->mounted);
		auto newChildWidget = build();
		this->child = updateChild(this->child, newChildWidget, this->index, this->depth + 1);
		Element::rebuild();
	}

	void ComponentElement::update(const WidgetPtr &newWidget) {
		Element::update(newWidget);
		beforeRebuild();
		rebuild();
	}

	void ComponentElement::unmount() {
		if (this->child) {
			this->child->unmount();
			this->child.reset();
		}
		Element::unmount();
	}

	void ComponentElement::updateIndex(size_t index) {
		Element::updateIndex(index);
		this->child->updateIndex(index);
	}

	// Stateless Element
	StatelessElement::StatelessElement(const StatelessWidgetPtr &widget) : ComponentElement(widget) {};

	Child StatelessElement::build() {
		auto widget = std::static_pointer_cast<StatelessWidget>(this->widget);
		assert(widget != nullptr);
		return widget->_build(*this);
	}

	// Stateful Element
	StatefulElement::StatefulElement(const StatefulWidgetPtr &widget) : ComponentElement(widget) {}

	Child StatefulElement::build() {
		assert(state != nullptr);
		return state->build(*this);
	}

	void StatefulElement::update(const WidgetPtr &newWidget) {
		this->state->setWidget(newWidget);
		ComponentElement::update(newWidget);
	}

	void StatefulElement::firstBuild() {
		this->state = std::static_pointer_cast<StatefulWidget>(widget)->_createState();

		this->state->element = this;
		this->state->setWidget(widget);
		this->state->initState();
		ComponentElement::firstBuild();
	}

	void StatefulElement::beforeRebuild() {
		this->state->widgetUpdated();
	}

	void StatefulElement::rebuild() {
		ComponentElement::rebuild();
	}

	void StatefulElement::unmount() {
		if (this->state) {
			this->state->dispose();
			this->state.reset();
		}
		ComponentElement::unmount();
	}

	// Render Object Element
	RenderObjectElement::RenderObjectElement(const RenderObjectWidgetPtr &widget) : Element(widget) {}

	void RenderObjectElement::mount(Element *parent, size_t index, size_t depth) {
		Element::mount(parent, index, depth);

		if (auto renderWidget = std::static_pointer_cast<RenderObjectWidget>(widget)) {
			renderObject = renderWidget->_createRenderObject();
			renderObject->element = this;
			attachRenderObject();
		}
	}

	void RenderObjectElement::update(const WidgetPtr &newWidget) {
		Element::update(newWidget);

		if (auto renderWidget = std::static_pointer_cast<RenderObjectWidget>(newWidget)) {
			renderObject->updateWidgetArgs(renderWidget->_getWidgetArgs());
			renderWidget->_updateRenderObject(renderObject.get());
		}
	}

	void RenderObjectElement::unmount() {
		detachRenderObject();
		renderObject.reset();
		Element::unmount();
	}

	void RenderObjectElement::updateIndex(size_t index) {
		Element::updateIndex(index);
		this->detachRenderObject();
		this->attachRenderObject();
	}

	RenderObjectElement *RenderObjectElement::getAncestorRenderObjectElement(Element *element) {
		Element *ancestor = element->parent;
		while (ancestor) {
			if (auto *roe = dynamic_cast<RenderObjectElement *>(ancestor)) {
				return roe;
			}
			if (ancestor->parent == ancestor) break;// Prevent potential infinite loop
			ancestor = ancestor->parent;
		}
		return nullptr;
	}

	void RenderObjectElement::attachRenderObject() {
		auto *ancestorElement = getAncestorRenderObjectElement(this);
		this->getApp()->needsRelayout = true;
		if (ancestorElement && ancestorElement->renderObject && this->renderObject) {
			ancestorElement->renderObject->addChild(this->renderObject, this->index);
		}
	}

	void RenderObjectElement::detachRenderObject() {
		auto *ancestorElement = getAncestorRenderObjectElement(this);
		this->getApp()->needsRelayout = true;
		if (ancestorElement && ancestorElement->renderObject && this->renderObject) {
			ancestorElement->renderObject->removeChild(this->renderObject);
		}
	}

	// Single Child Render Object Element
	void SingleChildRenderObjectElement::firstBuild() {
		auto childWidget = build();
		if (childWidget) {
			this->child = childWidget->_createElement();
			this->child->mount(this, 0, this->depth + 1);
		}
	}

	void SingleChildRenderObjectElement::mount(Element *parent, size_t index, size_t depth) {
		RenderObjectElement::mount(parent, index, depth);

		this->firstBuild();
	}

	void SingleChildRenderObjectElement::rebuild() {
		assert(this->mounted);
		auto newChildWidget = build();
		this->child = updateChild(this->child, newChildWidget, 0, this->depth + 1);
		RenderObjectElement::rebuild();
	}

	void SingleChildRenderObjectElement::update(const WidgetPtr &newWidget) {
		RenderObjectElement::update(newWidget);
		rebuild();
	}

	void SingleChildRenderObjectElement::unmount() {
		if (this->child) {
			this->child->unmount();
			this->child.reset();
		}
		RenderObjectElement::unmount();
	}

	// Multi Child Render Object Element
	void MultiChildRenderObjectElement::firstBuild() {
		auto childWidgets = buildAndPrune();
		for (size_t i = 0; i < childWidgets.size(); i++) {
			auto &childWidget = childWidgets.at(i);
			if (!childWidget) continue;
			auto element = childWidget->_createElement();
			this->children.push_back(element);
			element->mount(this, i, this->depth + 1);
		}
	}

	std::vector<Child> MultiChildRenderObjectElement::buildAndPrune() {
		auto childWidgets = build();
		childWidgets.erase(
			std::remove_if(childWidgets.begin(), childWidgets.end(), [](const Child &child) {
				return child == nullptr;
			}),
			childWidgets.end()
		);
		return childWidgets;
	}

	void MultiChildRenderObjectElement::mount(Element *parent, size_t index, size_t depth) {
		RenderObjectElement::mount(parent, index, depth);

		this->firstBuild();
	}

	void MultiChildRenderObjectElement::rebuild() {
		assert(this->mounted);
		auto newChildWidgets = buildAndPrune();
		updateChildren(this->children, newChildWidgets);
		RenderObjectElement::rebuild();
	}

	void MultiChildRenderObjectElement::update(const WidgetPtr &newWidget) {
		RenderObjectElement::update(newWidget);
		rebuild();
	}

	void MultiChildRenderObjectElement::unmount() {
		for (auto &child: this->children) {
			if (child) {
				child->unmount();
			}
		}
		this->children.clear();
		RenderObjectElement::unmount();
	}
}// namespace squi::core