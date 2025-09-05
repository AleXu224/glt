#include "element.hpp"

#include "widget.hpp"

namespace squi::core {
	StatefulElement::StatefulElement(const std::shared_ptr<Widget> &widget) : ComponentElement(widget) {
		this->state = widget->createState();

		this->state->element = this;
		this->state->setWidget(widget);
	};

	void ComponentElement::firstBuild() {
		this->child = widget->createElement();
		this->child->mount(this);
		this->child->rebuild();
	}
}// namespace squi::core