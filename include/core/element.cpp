#include "element.hpp"

#include "core/app.hpp"
#include "widget.hpp"

namespace squi::core {

	// Element
	App *Element::getApp() {
		assert(this->root != nullptr);
		auto root = dynamic_cast<RootWidget::Element *>(this->root);
		assert(root);
		return root->app;
	}

	void Element::markNeedsRebuild() {
		this->dirty = true;
		getApp()->dirtyElements.insert(this);
	}

	ElementPtr Element::updateChild(ElementPtr child, WidgetPtr newWidget) {
		if (!child)
			return nullptr;
		if (child->widget == newWidget) {
			// No change
			return child;
		}

		if (!newWidget) {
			if (child) {
				child->unmount();
			}
			return nullptr;
		}

		if (child && child->widget && Widget::canUpdate(child->widget, newWidget)) {
			// Same widget type, update in place
			child->update(newWidget);
			return child;
		} else {
			// Different widget type or null, replace
			if (child) {
				child->unmount();
			}
			if (newWidget) {
				auto newChild = newWidget->_createElement();
				newChild->mount(this);
				return newChild;
			} else {
				return nullptr;
			}
		}
	}

	void Element::updateChildren(std::vector<ElementPtr> &oldChildren, const std::vector<WidgetPtr> &newWidgets) {
		// FIXME: implement this
	}

	// Component Element
	void ComponentElement::firstBuild() {
		auto childWidget = build();
		if (childWidget) {
			this->child = childWidget->_createElement();
			this->child->mount(this);
		}
	}

	void ComponentElement::mount(Element *parent) {
		Element::mount(parent);

		this->firstBuild();
	}

	void ComponentElement::rebuild() {
		assert(this->mounted);
		auto newChildWidget = build();
		updateChild(this->child, newChildWidget);
		Element::rebuild();
	}

	void ComponentElement::update(const WidgetPtr &newWidget) {
		Element::update(newWidget);
		rebuild();
	}

	void ComponentElement::unmount() {
		if (this->child) {
			this->child->unmount();
			this->child.reset();
		}
		Element::unmount();
	}

	// Stateless Element
	StatelessElement::StatelessElement(const StatelessWidgetPtr &widget) : ComponentElement(widget) {};

	Child StatelessElement::build() {
		auto widget = std::static_pointer_cast<StatelessWidget>(this->widget);
		assert(widget != nullptr);
		return widget->_build(*this);
	}

	// Stateful Element
	StatefulElement::StatefulElement(const StatefulWidgetPtr &widget) : ComponentElement(widget) {
		this->state = widget->_createState();

		this->state->element = this;
		this->state->setWidget(widget);
		this->state->initState();
	}

	Child StatefulElement::build() {
		assert(state != nullptr);
		return state->build(*this);
	}

	void StatefulElement::update(const WidgetPtr &newWidget) {
		ComponentElement::update(newWidget);
		this->state->setWidget(newWidget);
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

	void RenderObjectElement::mount(Element *parent) {
		Element::mount(parent);

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

	RenderObjectElement *getAncestorRenderObjectElement(Element *element) {
		Element *ancestor = element->parent;
		while (ancestor) {
			if (auto roe = dynamic_cast<RenderObjectElement *>(ancestor)) {
				return roe;
			}
			if (ancestor->parent == ancestor) break;// Prevent potential infinite loop
			ancestor = ancestor->parent;
		}
		return nullptr;
	}

	void RenderObjectElement::attachRenderObject() {
		auto ancestorElement = getAncestorRenderObjectElement(this);
		if (ancestorElement && ancestorElement->renderObject && this->renderObject) {
			ancestorElement->renderObject->addChild(this->renderObject);
		}
	}

	void RenderObjectElement::detachRenderObject() {
		auto ancestorElement = getAncestorRenderObjectElement(this);
		if (ancestorElement && ancestorElement->renderObject && this->renderObject) {
			ancestorElement->renderObject->removeChild(this->renderObject);
		}
	}

	// Single Child Render Object Element
	void SingleChildRenderObjectElement::firstBuild() {
		auto childWidget = build();
		if (childWidget) {
			this->child = childWidget->_createElement();
			this->child->mount(this);
		}
	}

	void SingleChildRenderObjectElement::mount(Element *parent) {
		RenderObjectElement::mount(parent);

		this->firstBuild();
	}

	void SingleChildRenderObjectElement::rebuild() {
		assert(this->mounted);
		auto newChildWidget = build();
		updateChild(this->child, newChildWidget);
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
}// namespace squi::core