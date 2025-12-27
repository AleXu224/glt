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
		std::string text;

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
					ret.right = ret.left + element.size.x;
					ret.bottom = ret.top + element.size.y;
					ret = rect.dragInside(ret);
					return ret;
				},
				.child = SlideIn{
					.direction = Direction::bottom,
					.followChild = false,
					.child = Box{
						.widget{
							.width = Size::Wrap,
							.height = Size::Wrap,
							.sizeConstraints = BoxConstraints{
								.maxWidth = 600.f,
							},
							.padding = 8.f,
						},
						.color = Color::rgb(44, 44, 44),
						.borderColor = Color::black * 0.2f,
						.borderWidth = 1.f,
						.borderRadius = BorderRadius{4.f},
						.borderPosition = Box::BorderPosition::outset,
						.child = Text{
							.text = text,
							.lineWrap = true,
							.color = Color::white,
						},
					},
				},
			};
		}
	};

	[[nodiscard]] core::Child Tooltip::build(const Element &element) const {
		return Gesture{
			.onEnter = [this, &element](Gesture::State state) {
				auto renderObject = state.renderObject;
				const Key tooltipKey = std::make_shared<ValueKey>(std::format("tooltip_{}", reinterpret_cast<uintptr_t>(&element)));
				Navigator::of(element).pushOverlay(
					TooltipBox{
						.key = tooltipKey,
						.bounds = renderObject->getRect(),
						.text = text,
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
	core::Child TooltipWithTarget::State::build(const Element &element) {
		return Offset{
			.calculateContentBounds = [this](const Rect &rect, const SingleChildRenderObject &element) {
				auto *app = element.getApp();
				auto windowRect = app->rootRenderObject->getContentRect();

				auto renderObject = Element::getElementForGlobalKey(widget->targetKey);
				auto bounds = renderObject//
								? RenderObjectElement::getAncestorRenderObjectElement(renderObject.get())->renderObject->getRect()
								: Rect::fromPosSize(vec2{}, vec2{});

				Rect ret = rect;
				ret.left = bounds.left + (bounds.width() - element.size.x) / 2.f;
				if (bounds.top - element.size.y < windowRect.top) {
					// Not enough space above, show below
					ret.top = bounds.bottom;
					return ret;
				} else {
					ret.top = bounds.top - element.size.y;
				}
				ret.right = ret.left + element.size.x;
				ret.bottom = ret.top + element.size.y;
				ret = rect.dragInside(ret);
				return ret;
			},
			.child = SlideIn{
				.direction = Direction::bottom,
				.followChild = false,
				.child = Box{
					.widget{
						.width = Size::Wrap,
						.height = Size::Wrap,
						.sizeConstraints = BoxConstraints{
							.maxWidth = 600.f,
						},
						.padding = 8.f,
					},
					.color = Color::rgb(44, 44, 44),
					.borderColor = Color::black * 0.2f,
					.borderWidth = 1.f,
					.borderRadius = BorderRadius{4.f},
					.borderPosition = Box::BorderPosition::outset,
					.child = Text{
						.text = widget->text,
						.lineWrap = true,
						.color = Color::white,
					},
				},
			},
		};
	}
}// namespace squi