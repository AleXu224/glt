#include "widget.hpp"

namespace squi {
	struct Wrapper {
		// Args
		std::function<void(Widget &)> onInit{};
		std::function<void(Widget &)> afterInit{};
		std::function<void(Widget &)> onUpdate{};
		std::function<void(Widget &)> afterUpdate{};
		std::function<void(Widget &, vec2 &, vec2 &)> beforeLayout{};
		std::function<void(Widget &, vec2 &, vec2 &)> onLayout{};
		std::function<void(Widget &, vec2 &)> afterLayout{};
		std::function<void(Widget &, vec2 &)> onArrange{};
		std::function<void(Widget &)> beforeDraw{};
		std::function<void(Widget &)> onDraw{};
		std::function<void(Widget &)> afterDraw{};
		std::function<void(Widget &, std::shared_ptr<Widget>)> onChildAdded{};
		std::function<void(Widget &, std::shared_ptr<Widget>)> onChildRemoved{};
		Child child{};

		operator Child() const {
			if (child) {
				if (onInit) child->funcs().onInit.push_back(onInit);
				if (afterInit) child->funcs().afterInit.push_back(afterInit);
				if (onUpdate) child->funcs().onUpdate.push_back(onUpdate);
				if (afterUpdate) child->funcs().afterUpdate.push_back(afterUpdate);
				if (beforeLayout) child->funcs().beforeLayout.push_back(beforeLayout);
				if (onLayout) child->funcs().onLayout.push_back(onLayout);
				if (afterLayout) child->funcs().afterLayout.push_back(afterLayout);
				if (onArrange) child->funcs().onArrange.push_back(onArrange);
				if (beforeDraw) child->funcs().beforeDraw.push_back(beforeDraw);
				if (onDraw) child->funcs().onDraw.push_back(onDraw);
				if (afterDraw) child->funcs().afterDraw.push_back(afterDraw);
				if (onChildAdded) child->funcs().onChildAdded.push_back(onChildAdded);
				if (onChildRemoved) child->funcs().onChildRemoved.push_back(onChildRemoved);
			}
			return child;
		}
	};
}// namespace squi