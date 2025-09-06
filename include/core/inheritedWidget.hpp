#pragma once

#include "widget.hpp"
#include <typeindex>
#include <unordered_map>


namespace squi::core {
	struct InheritedWidget : Widget {
		virtual bool updateShouldNotify(const InheritedWidget *oldWidget) const = 0;
	};

	struct InheritedElement : ComponentElement {
		std::unordered_map<std::type_index, std::shared_ptr<InheritedWidget>> dependencies;

		InheritedElement(const WidgetPtr &widget) : ComponentElement(widget) {}

		void mount(Element *parent) override {
			ComponentElement::mount(parent);
			// Inherit dependencies from parent
			if (parent) {
				if (auto *inheritedParent = dynamic_cast<InheritedElement *>(parent)) {
					dependencies = inheritedParent->dependencies;
				}
			}
			// Add this widget if it's an InheritedWidget
			if (auto inheritedWidget = std::dynamic_pointer_cast<InheritedWidget>(widget)) {
				auto *rawPtr = inheritedWidget.get();
				std::type_index typeIndex(typeid(*rawPtr));
				dependencies[typeIndex] = inheritedWidget;
			}
		}

		template<typename T>
		std::shared_ptr<T> dependOnInheritedWidgetOfExactType() const {
			auto it = dependencies.find(std::type_index(typeid(T)));
			if (it != dependencies.end()) {
				return std::dynamic_pointer_cast<T>(it->second);
			}
			return nullptr;
		}

		void update(const WidgetPtr &newWidget) override {
			auto oldInheritedWidget = std::dynamic_pointer_cast<InheritedWidget>(widget);
			ComponentElement::update(newWidget);

			auto newInheritedWidget = std::dynamic_pointer_cast<InheritedWidget>(newWidget);
			if (newInheritedWidget) {
				bool shouldNotify = !oldInheritedWidget || newInheritedWidget->updateShouldNotify(oldInheritedWidget.get());

				auto *rawPtr = newInheritedWidget.get();
				std::type_index typeIndex(typeid(*rawPtr));
				dependencies[typeIndex] = newInheritedWidget;

				if (shouldNotify) {
					// Notify dependent elements that they need to rebuild
					notifyDependents();
				}
			}
		}

	private:
		void notifyDependents() {
			// In a real implementation, this would traverse the subtree and
			// mark elements that depend on this inherited widget as dirty
		}
	};
}// namespace squi::core
