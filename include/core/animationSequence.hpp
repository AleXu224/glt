#pragma once

#include "core/animated.hpp"
#include "core/curve.hpp"
#include <algorithm>
#include <chrono>
#include <functional>
#include <optional>
#include <vector>

namespace squi::core {
	using namespace std::chrono_literals;

	template<class T = float>
	struct AnimationSequence {
		struct Step {
			std::optional<T> to{};
			std::chrono::milliseconds duration = 200ms;
			std::function<float(float)> curve = Curve::easeOutCubic;
			std::function<void(float t)> onUpdate{};
			std::function<void()> onComplete{};
		};

		std::vector<Step> steps;
		T from{};
		std::function<void()> onComplete{};
		bool repeat = false;

		void mount(WidgetStateBase *state) {
			controller.element = state->element->weak_from_this();
			controller.app = state->element->getApp();
		}

		void run() {
			if (!controller.app) return;
			running = true;
			completedNotified = false;
			currentStep = 0;
			currentValue = from;
			totalDuration = std::chrono::milliseconds{0};
			for (const auto &step: steps) {
				totalDuration += step.duration;
			}
			startTime = std::chrono::steady_clock::now();
			controller.endTime = startTime + totalDuration;
			controller.run();
		}

		void stop() {
			if (!running) return;
			running = false;
			currentStep = steps.size();
			controller.endTime = controller.getFrameStartTime();
			if (!completedNotified) {
				completedNotified = true;
				if (onComplete) onComplete();
			}
		}

		[[nodiscard]] bool isRunning() const {
			return running;
		}

		[[nodiscard]] size_t stepIndex() const {
			return currentStep;
		}

		T getValue() {
			if (!running || !controller.app) return currentValue;
			if (steps.empty() || totalDuration <= std::chrono::milliseconds{0}) {
				running = false;
				if (!completedNotified) {
					completedNotified = true;
					if (onComplete) onComplete();
				}
				return currentValue;
			}

			auto now = controller.getFrameStartTime();
			auto elapsed = now - startTime;

			while (repeat && elapsed >= totalDuration) {
				elapsed -= totalDuration;
				startTime += totalDuration;
				currentStep = 0;
				currentValue = from;
				controller.endTime = startTime + totalDuration;
				controller.run();
			}

			auto remaining = elapsed;
			for (size_t i = 0; i < currentStep; ++i) {
				remaining -= steps.at(i).duration;
			}

			size_t idx = currentStep;
			while (idx < steps.size()) {
				const auto &step = steps.at(idx);
				auto stepDuration = step.duration;
				if (remaining < stepDuration) {
					auto t = std::chrono::duration<float>(remaining).count() / std::chrono::duration<float>(stepDuration).count();
					t = std::clamp(t, 0.f, 1.f);
					auto eased = step.curve ? step.curve(t) : t;
					if (step.onUpdate) step.onUpdate(eased);
					currentStep = idx;
					if (step.to.has_value()) {
						return Animator<T>::getValue(currentValue, *step.to, eased);
					}
					return currentValue;
				}
				if (step.onUpdate) step.onUpdate(1.f);
				if (step.onComplete) step.onComplete();
				if (step.to.has_value()) currentValue = *step.to;
				remaining -= stepDuration;
				idx++;
			}

			currentStep = steps.size();
			running = false;
			if (!completedNotified) {
				completedNotified = true;
				if (onComplete) onComplete();
			}
			return currentValue;
		}

	private:
		AnimatedController controller{};
		std::chrono::steady_clock::time_point startTime{};
		std::chrono::milliseconds totalDuration{};
		size_t currentStep = 0;
		bool running = false;
		bool completedNotified = false;
		T currentValue{};
	};
}// namespace squi::core
