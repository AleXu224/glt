#include "tooltip.hpp"
#include "core/app.hpp"
#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/navigator.hpp"
#include "widgets/offset.hpp"
#include "widgets/slideIn.hpp"
#include "widgets/text.hpp"


namespace squi {
	struct TooltipBox : StatelessWidget {
		// Args
		Key key;
		Rect bounds;

		[[nodiscard]] Child build(const Element &) const {
			return Offset{
				.calculateContentBounds = [this](const Rect &rect, const SingleChildRenderObject &element) {
					auto *app = element.getApp();
					auto windowRect = app->rootRenderObject->getContentRect();

					Rect ret = rect;
					ret.left = bounds.left + (bounds.width() - element.size.x) / 2.f;
					if (bounds.top - element.size.y < windowRect.top) {
						// Not enough space above, show below
						ret.top = bounds.bottom;
						return ret;
					} else {
						ret.top = bounds.top - element.size.y;
					}
					return ret;
				},
				.child = SlideIn{
					.direction = Direction::bottom,
					.followChild = false,
					.child = Box{
						.widget{
							.width = Size::Wrap,
							.height = Size::Wrap,
							.padding = 8.f,
						},
						.color = Color::black * 0.8f,
						.borderRadius = BorderRadius{4.f},
						.child = Text{
							.text = "Tooltip",
							.color = Color::white,
						},
					},
				},
			};
		}
	};

	[[nodiscard]] core::Child Tooltip::build(const Element &element) const {
		return Gesture{
			.onEnter = [&element](Gesture::State state) {
				auto renderObject = state.renderObject;
				const Key tooltipKey = std::make_shared<ValueKey>(std::format("tooltip_{}", reinterpret_cast<uintptr_t>(&element)));
				Navigator::of(element).pushOverlay(
					TooltipBox{
						.key = tooltipKey,
						.bounds = renderObject->getRect(),
					}
				);
			},
			.onLeave = [&element](Gesture::State state) {
				const Key tooltipKey = std::make_shared<ValueKey>(std::format("tooltip_{}", reinterpret_cast<uintptr_t>(&element)));
				Navigator::of(element).popOverlay(tooltipKey);
			},
			.child = child,
		};
	}
}// namespace squi