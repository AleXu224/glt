#include "widgets/toggleSwitch.hpp"
#include "box.hpp"
#include "button.hpp"
#include "container.hpp"
#include "row.hpp"
#include "text.hpp"
#include "wrapper.hpp"

using namespace squi;

namespace {
	struct ToggleSwitchTheme {
		Color borderColor;
		Color knobColor;
		Color knobBorderColor = knobColor;
		Color background;
		float knobWidth = 14.f;
	};
	const std::unordered_map<bool, std::unordered_map<ButtonState, ToggleSwitchTheme>> &toggleSwitchThemeData() {
		static std::unordered_map<bool, std::unordered_map<ButtonState, ToggleSwitchTheme>> data{
			{
				false,
				{
					{
						ButtonState::resting,
						ToggleSwitchTheme{
							.borderColor = Color::css(0xffffff, 0.6047f),
							.knobColor = Color::css(0xffffff, 0.786f),
							.knobBorderColor = Color::transparent,
							.background = Color::css(0x0, 0.1f),
						},
					},
					{
						ButtonState::hovered,
						ToggleSwitchTheme{
							.borderColor = Color::css(0xffffff, 0.6047f),
							.knobColor = Color::css(0xffffff, 0.786f),
							.background = Color::css(0xffffff, 0.0419f),
						},
					},
					{
						ButtonState::active,
						ToggleSwitchTheme{
							.borderColor = Color::css(0xffffff, 0.6047f),
							.knobColor = Color::css(0xffffff, 0.786f),
							.background = Color::css(0xffffff, 0.0698f),
							.knobWidth = 17.f,
						},
					},
					{
						ButtonState::disabled,
						ToggleSwitchTheme{
							.borderColor = Color::css(0xffffff, 0.1581f),
							.knobColor = Color::css(0xffffff, 0.3628f),
							.knobBorderColor = Color::transparent,
							.background = Color::transparent,
						},
					},
				},
			},
			{
				true,
				{
					{
						ButtonState::resting,
						ToggleSwitchTheme{
							.borderColor = Color::css(96, 205, 255),
							.knobColor = Color::black,
							.knobBorderColor = Color::css(0xffffff, 0.08f),
							.background = Color::css(96, 205, 255),
						},
					},
					{
						ButtonState::hovered,
						ToggleSwitchTheme{
							.borderColor = Color::css(96, 205, 255, 0.9f),
							.knobColor = Color::black,
							.knobBorderColor = Color::black,
							.background = Color::css(96, 205, 255, 0.9f),
						},
					},
					{
						ButtonState::active,
						ToggleSwitchTheme{
							.borderColor = Color::css(96, 205, 255, 0.8f),
							.knobColor = Color::black,
							.knobBorderColor = Color::black,
							.background = Color::css(96, 205, 255, 0.8f),
							.knobWidth = 17.f,
						},
					},
					{
						ButtonState::disabled,
						ToggleSwitchTheme{
							.borderColor = Color::css(0xffffff, 0.1581f),
							.knobColor = Color::css(0xffffff, 0.5302f),
							.knobBorderColor = Color::transparent,
							.background = Color::css(0xffffff, 0.1581f),
						},
					},
				},
			},
		};
		return data;
	};

	struct ToggleSwitchHead {
		// Args
		Observable<ToggleSwitch::ToggleSwitchState> stateEvent;
		Observable<ButtonState> buttonStateEvent;
		VoidObservable switchEvent;

		operator squi::Child() const {
			return GestureDetector{
				.onClick = [switchEvent = switchEvent](GestureDetector::Event) {
					switchEvent.notify();
				},
				.onUpdate = [buttonStateEvent = buttonStateEvent](GestureDetector::Event event) {
					if (event.state.focused)
						buttonStateEvent.notify(ButtonState::active);
					else if (event.state.hovered)
						buttonStateEvent.notify(ButtonState::hovered);
					else
						buttonStateEvent.notify(ButtonState::resting);
				},
				.child = Box{
					.widget{
						.width = 40.f,
						.height = 20.f,
						.padding = 3.f,
						.onInit = [stateEvent = stateEvent](Widget &w) {
							observe(w, stateEvent, [&w](ToggleSwitch::ToggleSwitchState state) {
								auto &box = w.as<Box::Impl>();
								auto &theme = toggleSwitchThemeData().at(state.active).at(state.state);
								box.setBorderColor(theme.borderColor);
								box.setColor(theme.background);
							});
						},
					},
					.color = Color::transparent,
					.borderColor = Color::black,
					.borderWidth = 1.f,
					.borderRadius = 10.f,
					.child = Row{
						.children{
							Container{
								.widget{
									.width = Size::Expand,
									.onInit = [stateEvent = stateEvent](Widget &w) {
										observe(w, stateEvent, [&w](ToggleSwitch::ToggleSwitchState state) {
											w.flags.visible = state.active;
										});
									},
								},
							},
							Box{
								.widget{
									.width = 14.f,
									.height = 14.f,
									.onInit = [stateEvent = stateEvent](Widget &w) {
										observe(w, stateEvent, [&w](ToggleSwitch::ToggleSwitchState state) {
											auto &box = w.as<Box::Impl>();
											[[maybe_unused]] auto col = Color::black;
											auto &theme = toggleSwitchThemeData().at(state.active).at(state.state);
											box.setBorderColor(theme.knobBorderColor);
											box.setColor(theme.knobColor);
											w.state.width = theme.knobWidth;
										});
									},
								},
								.color = Color::black,
								.borderWidth = 1.f,
								.borderRadius = 7.f,
							},
						},
					},
				},
			};
		}
	};

	struct ToggleSwitchStateText {
		// Args
		Observable<ToggleSwitch::ToggleSwitchState> stateEvent;

		operator squi::Child() const {
			return Text{
				.widget{
					.onInit = [stateEvent = stateEvent](Widget &w) {
						observe(w, stateEvent, [&w](ToggleSwitch::ToggleSwitchState state) {
							auto &text = w.as<Text::Impl>();
							if (state.active) {
								text.setText("On");
							} else {
								text.setText("Off");
							}
						});
					},
				},
				.text = "Off",
			};
		}
	};
}// namespace

ToggleSwitch::operator squi::Child() const {
	Observable<ToggleSwitchState> stateEvent;
	Observable<ButtonState> buttonStateEvent;
	VoidObservable switchEvent;

	auto stateText = ToggleSwitchStateText{
		.stateEvent = stateEvent,
	};
	auto getState = [](Widget &w) {
		auto &disabled = State::disabled.of(w);
		auto &active = State::active.of(w);
		auto &buttonState = State::state.of(w);
		ButtonState state = disabled ? ButtonState::disabled : buttonState;
		return ToggleSwitchState{.state = state, .active = active};
	};

	Child ret = Wrapper{
		.onInit = [switchEvent, stateEvent, buttonStateEvent, getState, onSwitch = onSwitch](Widget &w) {
			observe("buttonStateEvent", w, buttonStateEvent, [&w, getState, stateEvent](ButtonState state) {
				auto &buttonState = State::state.of(w);
				if (buttonState == state) return;
				buttonState = state;
				stateEvent.notify(getState(w));
			});
			observe("switchEvent", w, switchEvent, [&w, stateEvent, getState, onSwitch]() {
				auto &disabled = State::disabled.of(w);
				auto &active = State::active.of(w);
				if (disabled) return;
				active = !active;
				if (onSwitch) onSwitch(active);
				stateEvent.notify(getState(w));
			});
		},
		.afterInit = [stateEvent, getState](Widget &w) {
			stateEvent.notify(getState(w));
		},
		.child = Row{
			.widget = widget.withDefaultHeight(Size::Shrink).withDefaultWidth(Size::Shrink),
			.alignment = squi::Row::Alignment::center,
			.spacing = 12.f,
			.children{
				statePosition == StatePosition::before ? stateText : Child{},
				ToggleSwitchHead{
					.stateEvent = stateEvent,
					.buttonStateEvent = buttonStateEvent,
					.switchEvent = switchEvent,
				},
				statePosition == StatePosition::after ? stateText : Child{},
			},
		},
	};

	State::disabled.bind(ret, disabled);
	State::active.bind(ret, active);
	State::state.bind(ret, ButtonState::resting);
	State::stateEvent.bind(ret, stateEvent);

	return ret;
}
