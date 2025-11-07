#include "widgets/card.hpp"

#include "widgets/box.hpp"

namespace squi {

	[[nodiscard]] core::Child Card::build(const Element &) const {
		return Box{
			.widget = widget,
			.color{Color(0x2C2C2CF5)},
			.borderColor{Color(0x00000033)},
			.borderWidth{1.f},
			.borderRadius{8.f},
			.borderPosition = Box::BorderPosition::outset,
			.child = child,
		};
	}
}// namespace squi