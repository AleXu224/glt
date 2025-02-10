#include "rebuilder.hpp"
#include "container.hpp"
#include "registerEvent.hpp"

using namespace squi;

squi::Rebuilder::operator squi::Child() const {
	return RegisterEvent{
		.onInit = [rebuildEvent = rebuildEvent, buildFunc = buildFunc](Widget &w) {
			observe(w, rebuildEvent, [&w, buildFunc]() {
				if (buildFunc) w.setChildren({buildFunc()});
			});
		},
		.child = Container{
			.widget = widget.withDefaultWidth(Size::Wrap).withDefaultHeight(Size::Wrap),
			.child = buildFunc ? buildFunc() : nullptr,
		},
	};
}
