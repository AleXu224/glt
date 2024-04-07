#include "registerEvent.hpp"

using namespace squi;
squi::RegisterEvent::operator squi::Child() const {
	if (!child) return child;

	auto &funcs = child->funcs();

	if (onInit) funcs.onInit.emplace_back(onInit);
	if (onUpdate) funcs.onUpdate.emplace_back(onUpdate);
	if (afterUpdate) funcs.afterUpdate.emplace_back(afterUpdate);
	if (beforeLayout) funcs.beforeLayout.emplace_back(beforeLayout);
	if (onLayout) funcs.onLayout.emplace_back(onLayout);
	if (afterLayout) funcs.afterLayout.emplace_back(afterLayout);
	if (onArrange) funcs.onArrange.emplace_back(onArrange);
	if (afterArrange) funcs.afterArrange.emplace_back(afterArrange);
	if (beforeDraw) funcs.beforeDraw.emplace_back(beforeDraw);
	if (onDraw) funcs.onDraw.emplace_back(onDraw);
	if (afterDraw) funcs.afterDraw.emplace_back(afterDraw);
	if (onChildAdded) funcs.onChildAdded.emplace_back(onChildAdded);
	if (onChildRemoved) funcs.onChildRemoved.emplace_back(onChildRemoved);

    return child;
}
