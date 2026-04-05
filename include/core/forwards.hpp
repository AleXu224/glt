#pragma once

#include <memory>
#include <vector>


namespace squi::core {
	struct Widget;
	struct StatelessWidget;
	struct StatefulWidget;
	struct RenderObjectWidget;
	struct RenderObject;
	struct Element;
	struct Child;


	using Children = std::vector<Child>;
    using WidgetPtr = Child;
	using StatelessWidgetPtr = Child;
	using StatefulWidgetPtr = Child;
	using RenderObjectWidgetPtr = Child;
	using RenderObjectPtr = std::shared_ptr<RenderObject>;
	using Context = std::shared_ptr<Element>;
	using ElementPtr = std::shared_ptr<Element>;
	using ConstElementPtr = std::shared_ptr<const Element>;

	using InheritedMap = std::unordered_map<int64_t, Element *>;
}// namespace squi::core