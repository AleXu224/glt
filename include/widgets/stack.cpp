#include "widgets/stack.hpp"
#include "core/app.hpp"

namespace squi {
	// Iterates children in reverse order so that the children that are on top are iterated first
	// This is needed because otherwise you could click on a child that is behind another child
	void Stack::StackRenderObject::update() {
		auto &inputState = getApp()->inputState;
		size_t addedRects = 0;
		for (auto it = children.rbegin(); it != children.rend(); ++it) {
			auto &child = *it;
			child->update();
			inputState.g_hitCheckRects.push_back(child->getHitcheckRect());
			addedRects++;
		}
		for (size_t i = 0; i < addedRects; ++i) {
			inputState.g_hitCheckRects.pop_back();
		}
	}
}// namespace squi