#pragma once

#include "child.hpp"

namespace squi::core {
	struct Element;

	struct WidgetStateBase {
		Element *element;

		virtual ~WidgetStateBase() = default;

		virtual void initState() {}

		virtual Child build(const Element &element) = 0;
		virtual void setWidget(const std::shared_ptr<Widget> &newWidget) = 0;
	};

	template<class T>
	struct WidgetState : WidgetStateBase {
		const T *widget;

		void setWidget(const std::shared_ptr<Widget> &newWidget) override {
			this->widget = dynamic_cast<const T *>(newWidget.get());
			assert(this->widget != nullptr);
		}
	};
}// namespace squi::core