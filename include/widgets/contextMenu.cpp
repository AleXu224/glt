#include "contextMenu.hpp"

#include "widgets/box.hpp"
#include "widgets/offset.hpp"

namespace squi {
	[[nodiscard]] core::Child ContextMenu::build(const Element &) const {
		return Offset{
			.calculateContentBounds = [&](auto, const SingleChildRenderObject &renderObject) -> Rect {
				auto rect = renderObject.getLayoutRect();
				return Rect::fromPosSize(position, rect.size());
			},
			.child = Box{
				.widget{
					.width = 120.f,
					.height = 120.f,
				},
			},
		};
	}
}// namespace squi