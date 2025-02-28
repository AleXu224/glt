#include "rebuilder.hpp"
#include "container.hpp"
#include "registerEvent.hpp"

using namespace squi;

squi::Rebuilder::operator squi::Child() const {
	return RegisterEvent{
		.onInit = [rebuildEvent = rebuildEvent, buildFunc = buildFunc, onRebuild = onRebuild](Widget &w) {
			observe(w, rebuildEvent, [&w, buildFunc, onRebuild]() {
				if (buildFunc) w.setChildren({buildFunc()});
				if (onRebuild) onRebuild();
			});
		},
		.child = Container{
			.widget = widget.withDefaultWidth(Size::Wrap).withDefaultHeight(Size::Wrap),
			.child = buildFunc ? buildFunc() : nullptr,
		},
	};
}
