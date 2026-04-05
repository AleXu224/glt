#pragma once

#include "core/core.hpp"
#include "core/widget.hpp"


namespace squi::core {
	template<class T>
	concept HasContext = requires(T &&t) {
		typename std::remove_cvref_t<T>::Context;
		{ std::declval<typename std::remove_cvref_t<T>::Context>().widget } -> std::same_as<const T * &&>;
	};

	template<class T>
	struct InheritedWidget : StatelessWidget {
		Child build(const Element &element) const {
			return static_cast<const T *>(this)->child;
		}

		struct Element : core::StatelessElement {
			using ContextType = typename T::Context;
			static_assert(HasContext<T>, "InheritedWidget requires a Context");
			ContextType context;
			InheritedMap inheritedMapCopy;

			Element(const StatelessWidgetPtr &widget) : StatelessElement(widget), context(static_cast<const T *>(widget.get())) {}

			void update(const WidgetPtr &newWidget) override {
				context.widget = static_cast<const T *>(newWidget.widget.get());
				StatelessElement::update(newWidget);
			}

			void mount(core::Element *parent, size_t index, size_t depth) override {
				this->inheritedMapCopy = *parent->inheritedMap;
				this->inheritedMap = &this->inheritedMapCopy;
				this->inheritedMap->emplace(this->widget->getTypeHash(), this);
				StatelessElement::mount(parent, index, depth);
			}

			Child build() override {
				if (auto inheritedWidget = std::static_pointer_cast<InheritedWidget<T>>(widget)) {
					return inheritedWidget->_build(*this);
				}
				return nullptr;
			}
		};

		static auto of(const core::Element &element) {
			if (auto it = element.inheritedMap->find(typeid(T).hash_code()); it != element.inheritedMap->end()) {
				return &static_cast<Element *>(it->second)->context;
			}
			return static_cast<typename T::Context *>(nullptr);
		}

		static auto of(const WidgetStateBase *state) {
			return of(*state->element);
		}
	};

}// namespace squi::core