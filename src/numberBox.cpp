#include "numberBox.hpp"

#include "inputState.hpp"
#include "textBox.hpp"
#include "wrapper.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>


using namespace squi;

NumberBox::operator Child() const {
	auto storage = std::make_shared<Storage>(Storage{
		.value = value,
		.min = min,
		.max = max,
		.step = step,
	});

	auto formatVal = [storage, formatter = formatter]() {
		if (formatter) return formatter(storage->value);

		return std::format("{}", storage->value);
	};

	storage->onChange = [onChange = onChange, storage = std::weak_ptr<Storage>(storage)](float val) {
		if (storage.expired()) return;
		auto &storageRef = *storage.lock();
		storageRef.value = val;
		storageRef.clampValue();
		if (onChange) onChange(storageRef.value);
	};

	return Wrapper{
		.onInit = [storage, stateObserver = controller.stateObserver, updateText = controller.updateText, valueUpdater = valueUpdater, formatVal](Widget &w) {
			auto refreshVal = [storage, updateText, formatVal]() {
				auto lastValue = storage->value;
				storage->clampValue();
				if (lastValue != storage->value) {
					storage->onChange(storage->value);
					updateText.notify(formatVal());
				}
			};
			State::min.bind(
				w,
				decltype(State::min)::Type(
					[refreshVal](Widget &, const float &) {
						refreshVal();
					},
					&w, Ref(storage->min)
				)
			);
			State::max.bind(
				w,
				decltype(State::max)::Type(
					[refreshVal](Widget &, const float &) {
						refreshVal();
					},
					&w, Ref(storage->max)
				)
			);
			State::step.bind(w, storage->step);
			w.customState.add("_stateObservable", stateObserver.observe([storage](TextBox::InputState state) {
				storage->focused = (state == TextBox::InputState::focused);
			}));
			w.customState.add(valueUpdater.observe([updateText, formatVal, storage](float newVal) {
				storage->value = newVal;
				updateText.notify(formatVal());
			}));
		},
		.onUpdate = [storage, updateText = controller.updateText, formatVal](Widget &w) {
			if (!storage->focused) return;
			auto &inputState = InputState::of(&w);
			if (inputState.isKeyPressedOrRepeat(GLFW_KEY_UP)) {
				storage->value += storage->step;
				storage->clampValue();
				updateText.notify(formatVal());
			}
			if (inputState.isKeyPressedOrRepeat(GLFW_KEY_DOWN)) {
				storage->value -= storage->step;
				storage->clampValue();
				updateText.notify(formatVal());
			}
		},
		.child = TextBox{
			.widget{widget},
			.disabled = disabled,
			.text = formatVal(),
			.controller{
				.disable = controller.disable,
				.focus = controller.focus,
				.onChange = [onChange = onChange, controllerOnChange = controller.onChange, storage](std::string_view newText) {
					if (controllerOnChange) controllerOnChange(newText);
					float val = std::stof(std::string{newText});
					storage->value = val;
					if (onChange) onChange(val);
				},
				.onSubmit = controller.onSubmit,
				.validator = [storage, textValidator = controller.validator, numValidator = validator](std::string_view str) -> TextBox::Controller::ValidatorResponse {
					if (textValidator) {
						auto _ = textValidator(str);
						if (!_.valid) return _;
					}
					float val{};
					try {
						val = std::stof(std::string(str));
					} catch (const std::invalid_argument &) {
						return {.valid = false, .message = "Invalid number"};
					} catch (const std::out_of_range &) {
						return {.valid = false, .message = "Number too big/small"};
					}

					if (val < storage->min) return {.valid = false, .message = "Value is too small"};
					if (val > storage->max) return {.valid = false, .message = "Value is too big"};

					if (numValidator) {
						auto _ = numValidator(val);
						if (!_.valid) return _;
					}

					return {.valid = true};
				},
				.updateText = controller.updateText,
				.stateObserver = controller.stateObserver,
				.selectAll = controller.selectAll,
			},
		},
	};
}

void squi::NumberBox::Storage::clampValue() {
	value = std::clamp(value, min, max);
}
