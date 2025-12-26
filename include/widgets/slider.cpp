#include "slider.hpp"
#include "theme.hpp"
#include "widgets/animatedBox.hpp"
#include "widgets/box.hpp"
#include "widgets/gestureDetector.hpp"
#include "widgets/layoutBuilder.hpp"
#include "widgets/navigator.hpp"
#include "widgets/offset.hpp"
#include "widgets/row.hpp"
#include "widgets/stack.hpp"

#include "utils.hpp"
#include "widgets/tooltip.hpp"

namespace squi {
	float Slider::State::getHandleInnerSize() const {
		if (focused) return 10.f;
		if (handleHovered) return 14.f;
		return 12.f;
	}

	void Slider::State::createOrUpdateTooltip() {
		Navigator::of(*this->element).pushOrReplaceOverlay(TooltipWithTarget{
			.key = tooltipKey,
			.targetKey = handleKey,
			.text = std::format("{}", widget->value),
		});
	}

	core::Child Slider::State::build(const Element &element) {
		auto newWidget = widget->widget;
		newWidget.height = newWidget.height.value_or(32.f);

		auto accentColor = ThemeManager::getTheme().accent;

		constexpr float handleSize = 20.f;
		constexpr float barPadding = 4.f;
		const float handleInnerSize = getHandleInnerSize();

		auto ticks = computeTicks();

		return Gesture{
			.onFocus = [this](Gesture::State state) {
				setState([&]() {
					focused = true;
				});
				createOrUpdateTooltip();
			},
			.onFocusLoss = [this](Gesture::State state) {
				Navigator::of(*this->element).popOverlay(tooltipKey);
				setState([&]() {
					focused = false;
				});
			},
			.onDrag = [this](Gesture::State state) {
				auto cursorPosRelative = state.getCursorPos() - state.renderObject->pos - vec2{handleSize / 2.f, 0.f};
				float percent = cursorPosRelative.x / (state.renderObject->size.x - handleSize);
				percent = std::clamp(percent, 0.f, 1.f);
				float newValue = widget->minValue + percent * (widget->maxValue - widget->minValue);
				if (widget->onChange) {
					widget->onChange(newValue);
				}
			},
			.child = Stack{
				.widget = newWidget,
				.children{
					// Bar background
					Row{
						.widget{
							.padding = Padding{barPadding, 0.f},
						},
						.crossAxisAlignment = Row::Alignment::center,
						.children{
							LayoutBuilder{
								.builder = [this, accentColor](const BoxConstraints &constraints) -> Child {
									float percent = (widget->value - widget->minValue) / (widget->maxValue - widget->minValue);
									float total = constraints.maxWidth + barPadding * 2;
									float progressWidth = percent * (total - handleSize);
									float barWidth = progressWidth + (handleSize / 2.f) - barPadding;

									return Box{
										.widget{
											.width = barWidth,
											.height = 4.f,
										},
										.color = accentColor,
										.borderRadius = BorderRadius{2.f}.withRight(0.f),
									};
								},
							},
							Box{
								.widget{
									.height = 4.f,
								},
								.color = Color::rgba(255, 255, 255, 0.5442),
								.borderRadius = BorderRadius{2.f}.withLeft(0.f),
							},
						},
					},
					// Ticks
					ticks.empty()//
						? Child{}
						: LayoutBuilder{
							  .builder = [this, ticks](const BoxConstraints &constraints) -> Child {
								  Children tickChildren;
								  for (float tickValue: ticks) {
									  float percent = (tickValue - widget->minValue) / (widget->maxValue - widget->minValue);
									  float tickX = percent * (constraints.maxWidth - handleSize) + handleSize / 2.f;

									  tickChildren.push_back(Offset{
										  .calculateContentBounds = [tickX](const Rect &rect, const SingleChildRenderObject &) {
											  auto ret = rect;
											  ret.left += tickX;
											  return ret;
										  },
										  .child = Box{
											  .widget = {
												  .width = 1.f,
												  .height = 4.f,
												  .alignment = Alignment::BottomLeft,
												  .margin = Margin{}.withBottom(6.f),
											  },
											  .color = Color::rgba(255, 255, 255, 0.6047),
										  },
									  });
								  }
								  return Stack{
									  .children = tickChildren,
								  };
							  },
						  },
					// Handle
					LayoutBuilder{
						.builder = [this, accentColor, handleSize, handleInnerSize](const BoxConstraints &constraints) -> Child {
							float percent = (widget->value - widget->minValue) / (widget->maxValue - widget->minValue);
							float handleX = percent * (constraints.maxWidth - handleSize) + handleSize / 2.f;

							return Gesture{
								.onEnter = [this](Gesture::State state) {
									setState([&]() {
										handleHovered = true;
									});
								},
								.onLeave = [this](Gesture::State state) {
									setState([&]() {
										handleHovered = false;
									});
								},
								.child = Offset{
									.calculateContentBounds = [handleX](const Rect &rect, const SingleChildRenderObject &) {
										auto ret = rect;
										ret.left += handleX - (handleSize / 2.f);
										return ret;
									},
									.child = Box{
										.key = handleKey,
										.widget = {
											.width = handleSize,
											.height = handleSize,
											.alignment = Alignment::CenterLeft,
										},
										.color = Color::rgba(69, 69, 69, 1),
										.borderRadius = BorderRadius{handleSize / 2.f},
										.child = AnimatedBox{
											.widget{
												.width = handleInnerSize,
												.height = handleInnerSize,
												.alignment = Alignment::Center,
											},
											.color = accentColor,
											.borderRadius = BorderRadius{handleInnerSize / 2.f},
										},
									},
								},
							};
						},
					},
				},
			},
		};
	}
	std::vector<float> Slider::State::computeTicks() const {
		std::vector<float> result;
		std::visit(
			utils::overloaded{
				[](const std::monostate &) {},
				[&](const TickCount &tickCount) {
					if (tickCount.count == 0) return;
					if (tickCount.count == 1) {
						result.push_back((widget->minValue + widget->maxValue) / 2.f);
						return;
					}
					float interval = (widget->maxValue - widget->minValue) / (tickCount.count - 1);
					for (uint32_t i = 0; i < tickCount.count; i++) {
						result.push_back(widget->minValue + i * interval);
					}
				},
				[&](const TickInterval &tickInterval) {
					if (tickInterval.interval <= 0.f) return;
					for (float val = widget->minValue + tickInterval.offset; val <= widget->maxValue; val += tickInterval.interval) {
						if (val >= widget->minValue) {
							result.push_back(val);
						}
					}
				},
				[&](const std::vector<float> &ticks) {
					result = ticks;
				},
			},
			widget->ticks
		);
		return result;
	}
}// namespace squi