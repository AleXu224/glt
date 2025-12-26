#pragma once

#include "memory"
#include <vector>

namespace squi::core {
	struct Widget;
	struct StatelessWidget;
	struct StatefulWidget;
	struct RenderObjectWidget;
	struct RenderObject;
	struct Element;

	using Child = std::shared_ptr<Widget>;
	using Children = std::vector<Child>;
	using WidgetPtr = std::shared_ptr<Widget>;
	using StatelessWidgetPtr = std::shared_ptr<StatelessWidget>;
	using StatefulWidgetPtr = std::shared_ptr<StatefulWidget>;
	using RenderObjectWidgetPtr = std::shared_ptr<RenderObjectWidget>;
	using RenderObjectPtr = std::shared_ptr<RenderObject>;
	using Context = std::shared_ptr<Element>;
	using ElementPtr = std::shared_ptr<Element>;
	using ConstElementPtr = std::shared_ptr<const Element>;
}// namespace squi::core