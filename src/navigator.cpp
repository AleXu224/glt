#include "navigator.hpp"
#include "stack.hpp"
#include "widget.hpp"

using namespace squi;

void squi::Navigator::Controller::push(const Child &child) const {
	pusher.notify(child);
}

void squi::Navigator::Controller::pop() const {
	popper.notify();
}

squi::Navigator::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();
	Observable<size_t> isVisible{};
	Observable<size_t> selfDestruct{};

	Child ret = Stack{
		.widget = widget,
	};

	ret->funcs().onInit.emplace_back([child = child, controller = controller, storage, isVisible, selfDestruct](Widget &w) {
		w.customState.add(controller.pusher.observe([selfDestruct, isVisible, storage, wPtr = w.weak_from_this()](const Child &child) {
			if (!child) return;
			if (auto w = wPtr.lock()) {
				auto id = storage->lastId++;

				child->customState.add("NavigatorHelperIsVisible", isVisible.observe([id, childPtr = child->weak_from_this()](size_t testId) {
					if (auto child = childPtr.lock()) {
						child->flags.visible = (id == testId);
					}
				}));
				child->customState.add("NavigatorHelperSelfDestruct", selfDestruct.observe([id, childPtr = child->weak_from_this()](size_t testId) {
					if (auto child = childPtr.lock()) {
						if (id == testId) {
							child->deleteLater();
						}
					}
				}));

				storage->ids.emplace_back(id);

				w->addChild(child);

				isVisible.notify(id);
			}
		}));

		w.customState.add(controller.popper.observe([isVisible, selfDestruct, storage, wPtr = w.weak_from_this()]() {
			if (auto w = wPtr.lock() && !storage->ids.empty()) {
				selfDestruct.notify(storage->ids.back());
				storage->ids.pop_back();
				if (!storage->ids.empty()) isVisible.notify(storage->ids.back());
			}
		}));

		controller.push(child);
	});


	return ret;
}
