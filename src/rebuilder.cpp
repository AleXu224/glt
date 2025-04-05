#include "rebuilder.hpp"
#include "container.hpp"
#include "wrapper.hpp"

using namespace squi;

squi::Rebuilder::operator squi::Child() const {
	return Wrapper{
		.onInit = [rebuildEvent = rebuildEvent, buildFunc = buildFunc, onRebuild = onRebuild](Widget &w) {
			static uint32_t id = 0;
			auto name = std::format("rebuilder_{}", id++);
			observe(name, w, rebuildEvent, [&w, buildFunc, onRebuild]() {
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
