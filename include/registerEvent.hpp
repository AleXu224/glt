#pragma once
#include "widget.hpp"

namespace squi {
	struct RegisterEvent {
		// Args
		std::function<void(Widget &)> onInit{};
		std::function<void(Widget &)> onUpdate{};
		std::function<void(Widget &)> afterUpdate{};
		std::function<void(Widget &, vec2 &, vec2 &)> beforeLayout{};
		std::function<void(Widget &, vec2 &, vec2 &)> onLayout{};
		std::function<void(Widget &, vec2 &)> afterLayout{};
		std::function<void(Widget &, vec2 &)> onArrange{};
		std::function<void(Widget &, vec2 &)> afterArrange{};
		std::function<void(Widget &)> beforeDraw{};
		std::function<void(Widget &)> onDraw{};
		std::function<void(Widget &)> afterDraw{};
		std::function<void(Widget &, std::shared_ptr<Widget>)> onChildAdded{};
		std::function<void(Widget &, std::shared_ptr<Widget>)> onChildRemoved{};

		Child child{};

		operator squi::Child() const;
	};
}// namespace squi