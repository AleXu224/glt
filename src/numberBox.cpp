#include "numberBox.hpp"
#include "gestureDetector.hpp"
#include "registerEvent.hpp"
#include "textBox.hpp"
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
	storage->onChange = [onChange = onChange, storage = std::weak_ptr<Storage>(storage)](float val) {
		if (storage.expired()) return;
		auto &storageRef = *storage.lock();
		storageRef.value = val;
		storageRef.clampValue();
		if (onChange) onChange(storageRef.value);
	};


	return RegisterEvent{
		.onInit = [storage, stateObserver = controller.stateObserver](Widget &w){
			w.customState.add("_stateObservable", stateObserver.observe([storage](TextBox::InputState state){
				storage->focused = (state == TextBox::InputState::focused);
			}));
		},
		.onUpdate = [storage, updateText = controller.updateText](Widget &) {
			if (!storage->focused) return;
			if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_UP)) {
				storage->value += storage->step;
				storage->clampValue();
				updateText.notify(std::format("{}", storage->value));
			}
			if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_DOWN)) {
				storage->value -= storage->step;
				storage->clampValue();
				updateText.notify(std::format("{}", storage->value));
			}
		},
		.child = TextBox{
			.widget{widget},
			.disabled = disabled,
			.text = std::format("{}", value),
			.controller{
				.disable = controller.disable,
				.focus = controller.focus,
				.onChange = controller.onChange,
				.onSubmit = controller.onSubmit,
				.validator = [storage, textValidator = controller.validator, numValidator = validator](std::string_view str) -> TextBox::Controller::ValidatorResponse {
					if (textValidator) {
						auto _ = textValidator(str);
						if (!_.valid) return _;
					}
					try {
						auto _ = std::stof(std::string(str));
						if (_ < storage->min) return {.valid = false, .message = "Value is too small"};
						if (_ > storage->max) return {.valid = false, .message = "Value is too big"};
						if (storage->onChange) storage->onChange(_);
					} catch (const std::invalid_argument &) {
						return {.valid = false, .message = "Invalid number"};
					} catch (const std::out_of_range &) {
						return {.valid = false, .message = "Number too big/small"};
					}
					
					if (numValidator) {
						auto _ = numValidator(storage->value);
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
