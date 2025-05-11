#include "widgets/slider.hpp"
#include "algorithm"
#include "align.hpp"
#include "box.hpp"
#include "gestureDetector.hpp"
#include "rebuilder.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "theme.hpp"


using namespace squi;

struct SliderTick {
	// Args
	float minVal;
	float maxVal;
	float val;

	operator squi::Child() const {
		return Box{
			.widget{

			},
		};
	}
};

Slider::operator squi::Child() const {
	auto theme = ThemeManager::getTheme();

	static StateInfo<ChildRef> tooltip{.name = "tooltip"};
	VoidObservable recalcTicks{};
	Observable<float> updateTooltip;

	Child ret = Stack{
		.widget = widget.withDefaultHeight(32.f)
					  .withDefaultPadding({4.f, 0.f}),
		.children{
			Align{
				.child = Rebuilder{
					.rebuildEvent = recalcTicks,
					.buildFunc = []() {
						return Stack{
							.widget{
								.height = Size::Shrink,
								.padding = Padding{}.withTop(16.f),
								.onInit = [](Widget &w) {
									auto min = *State::minVal.of(w.state.parent->state.parent);
									auto max = *State::maxVal.of(w.state.parent->state.parent);
									auto step = *State::step.of(w.state.parent->state.parent);

									auto knobWidth = 22.f;

									if (!step) return;
									std::vector<float> ticks{min, max};
									for (float v = min + step.value(); v < max; v += step.value()) {
										ticks.emplace_back(v);
									}

									Children ret;
									ret.reserve(ticks.size());
									for (const auto &tick: ticks) {
										ret.emplace_back(Box{
											.widget{
												.width = 1.f,
												.height = 4.f,
												.onArrange = [=](Widget &w, vec2 &pos) {
													auto availableWidth = w.state.parent->getContentRect().width() - knobWidth;
													auto position_p = (tick - min) / (max - min);
													auto offset = (availableWidth * position_p) + knobWidth / 2.f;
													pos.x += offset;
												},
											},
											.color = Color::css(255, 255, 255, 0.6047),
										});
									}
									w.setChildren(ret);
								},
							},
						};
					},
				},
			},
			Align{
				.child = Box{
					.widget{
						.height = 4.f,
					},
					.color = Color::css(255, 255, 255, 0.5442f),
					.borderColor = Color::css(255, 255, 255, 0.6047f),
					.borderWidth = 1.f,
					.borderRadius = 2.f,
					.borderPosition = Box::BorderPosition::outset,
					.child = Box{
						.widget{
							.onLayout = [](Widget &w, vec2 &maxSize, vec2 &minSize) {
								auto min = *State::minVal.of(w.state.parent->state.parent);
								auto max = *State::maxVal.of(w.state.parent->state.parent);
								auto step = *State::step.of(w.state.parent->state.parent);
								auto val = *State::value.of(w.state.parent->state.parent);
								auto newVal = calcValue(min, max, step, val);
								auto p = (newVal - min) / (max - min);
								maxSize.x *= p;
							},
						},
						.color = theme.accent,
						.borderRadius = 2.f,
					},
				},
			},
			Align{
				.xAlign = 0.f,
				.child = GestureDetector{
					.onFocus = [updateTooltip](GestureDetector::Event event) {
						auto &ref = tooltip.of(event.widget);
						if (auto w = ref.lock()) {
							w->deleteLater();
							ref.reset();
						}

						auto min = *State::minVal.of(event.widget.state.parent);
						auto max = *State::maxVal.of(event.widget.state.parent);
						auto step = *State::step.of(event.widget.state.parent);
						auto value = *State::value.of(event.widget.state.parent);
						auto newVal = calcValue(min, max, step, value);

						Child overlay = Box{
							.widget{
								.width = Size::Shrink,
								.height = Size::Shrink,
								.margin = Margin{0.f, 4.f},
								.padding = Padding{8.f, 6.f},
								.onArrange = [target = event.widget.weak_from_this()](Widget &w, vec2 &pos) {
									if (auto t = target.lock()) {
										pos = t->getPos()
												  .withYOffset(-w.getLayoutSize().y)
												  .withXOffset(t->getSize().x / 2.f - w.getLayoutSize().x / 2.f);
									}
								},
							},
							.color = Color::css(44, 44, 44, 0.96),
							.borderColor = Color::black * 0.2f,
							.borderWidth = 1.f,
							.borderRadius = 4.f,
							.borderPosition = Box::BorderPosition::outset,
							.child = Text{
								.widget{
									.onInit = [updateTooltip](Widget &w) {
										observe(w, updateTooltip, [&w](float val) {
											auto &text = w.as<Text::Impl>();
											text.setText(std::format("{}", val));
										});
									},
								},
								.text = std::format("{}", newVal),
							},
						};
						event.widget.addOverlay(overlay);
						ref = overlay;
					},
					.onFocusLoss = [](GestureDetector::Event event) {
						auto ref = tooltip.of(event.widget);
						if (auto w = ref.lock()) {
							w->deleteLater();
							ref.reset();
						}
					},
					.onDrag = [](GestureDetector::Event event) {
						auto offset = event.state.getCursorPos().x;
						auto knobWidth = event.widget.getRect().width();
						auto availableWidth = event.widget.state.parent->getContentRect().width() - knobWidth;
						auto parentContentPos = event.widget.state.parent->getContentPos().x + knobWidth / 2.f;
						auto position_p = (offset - parentContentPos) / availableWidth;
						position_p = std::clamp(position_p, 0.f, 1.f);

						auto min = *State::minVal.of(event.widget.state.parent);
						auto max = *State::maxVal.of(event.widget.state.parent);
						auto step = *State::step.of(event.widget.state.parent);
						auto newVal = calcValue(min, max, step, min + (max - min) * position_p);
						State::value.of(event.widget.state.parent) = newVal;
					},
					.child = Box{
						.widget{
							.width = 22.f,
							.height = 22.f,
							.padding = 5.f,
							.onInit = [](Widget &w) {
								tooltip.bind(w, {});
							},
							.onArrange = [](Widget &w, vec2 &pos) {
								auto availableWidth = w.state.parent->getContentRect().width();
								auto min = *State::minVal.of(w.state.parent);
								auto max = *State::maxVal.of(w.state.parent);
								auto step = *State::step.of(w.state.parent);
								auto val = *State::value.of(w.state.parent);
								auto newVal = calcValue(min, max, step, val);
								auto p = (newVal - min) / (max - min);
								pos.x += (availableWidth - w.getRect().width()) * p;
							},
						},
						.color = Color::css(69, 69, 69),
						.borderColor = Color::css(255, 255, 255, 0.093f),
						.borderWidth = 1.f,
						.borderRadius = 22.f / 2.f,
						.child = Box{
							.color = theme.accent,
							.borderRadius = 6.f,
						},
					},
				},
			}
		},
	};


	State::value.bind(
		ret,
		Stateful<float, StateImpact::RelayoutNeeded>(
			[valueChanged = valueChanged, updateTooltip](Widget &w, const float &val) {
				auto retVal = calcValue(
					State::minVal.of(w),
					State::maxVal.of(w),
					State::step.of(w),
					val
				);
				if (valueChanged) valueChanged(
					retVal
				);
				updateTooltip.notify(retVal);
			},
			ret.get(),
			value
		)
	);
	State::minVal.bind(
		ret,
		Stateful<float, StateImpact::RelayoutNeeded>(
			[valueChanged = valueChanged, recalcTicks](Widget &w, const float &val) {
				if (valueChanged) valueChanged(
					calcValue(
						val,
						State::maxVal.of(w),
						State::step.of(w),
						State::value.of(w)
					)
				);
				recalcTicks.notify();
			},
			ret.get(),
			minVal
		)
	);
	State::maxVal.bind(
		ret,
		Stateful<float, StateImpact::RelayoutNeeded>(
			[valueChanged = valueChanged, recalcTicks](Widget &w, const float &val) {
				if (valueChanged) valueChanged(
					calcValue(
						State::minVal.of(w),
						val,
						State::step.of(w),
						State::value.of(w)
					)
				);
				recalcTicks.notify();
			},
			ret.get(),
			maxVal
		)
	);
	State::step.bind(
		ret,
		Stateful<std::optional<float>, StateImpact::RelayoutNeeded>(
			[valueChanged = valueChanged, recalcTicks](Widget &w, const std::optional<float> &val) {
				if (valueChanged) valueChanged(
					calcValue(
						State::minVal.of(w),
						State::maxVal.of(w),
						val,
						State::value.of(w)
					)
				);
				recalcTicks.notify();
			},
			ret.get(),
			step
		)
	);


	return ret;
}

float squi::Slider::calcValue(float min, float max, std::optional<float> step, float val) {
	auto clamped = std::clamp(val, min, max);
	if (step) {
		auto relative = clamped - min;
		auto nearestStep = std::round(relative / step.value());
		return min + nearestStep * step.value();
	}
	return clamped;
}
