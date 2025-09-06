#include "state.hpp"

#include "element.hpp"

namespace squi::core {
	void WidgetStateBase::setState(std::function<void()> fn) {
		if (fn) {
			fn();
		}
		if (element) {
			element->markNeedsRebuild();
		}
	}
}// namespace squi::core