#pragma once

#include "child.hpp"
#include <functional>

namespace squi::core {
	struct Element;

	struct WidgetStateBase {
		Element *element = nullptr;

		virtual ~WidgetStateBase() = default;

		virtual void initState() {}
		virtual void dispose() {}

		virtual Child build(const Element &element) = 0;
		virtual void setWidget(const std::shared_ptr<Widget> &newWidget) = 0;

		void setState(std::function<void()> fn = nullptr);
	};

	template<class T>
	struct WidgetState : WidgetStateBase {
		const T *widget = nullptr;

		void setWidget(const std::shared_ptr<Widget> &newWidget) override {
			this->widget = dynamic_cast<const T *>(newWidget.get());
			assert(this->widget != nullptr);
		}
	};
}// namespace squi::core