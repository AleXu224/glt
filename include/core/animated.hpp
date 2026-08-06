#pragma once

#include "borderRadius.hpp"
#include "borderWidth.hpp"
#include "color.hpp"
#include "core/animationController.hpp"
#include "core/core.hpp"
#include "core/curve.hpp"
#include "vec2.hpp"
#include <algorithm>
#include <chrono>

namespace squi::core {
	using namespace std::chrono_literals;

	struct AnimatedController : AnimationController {
		using AnimationController::AnimationController;

		AnimatedController(const AnimatedController &) = delete;
		AnimatedController(AnimatedController &&) noexcept = delete;
		AnimatedController &operator=(const AnimatedController &) = delete;
		AnimatedController &operator=(AnimatedController &&) noexcept = delete;

		std::weak_ptr<Element> element = {};
		App *app = nullptr;

		[[nodiscard]] bool isCompleted() const override;

		void markElementDirty() override {
			if (auto elementPtr = element.lock()) {
				elementPtr->markNeedsRebuild();
			}
		}

		void run();

		[[nodiscard]] std::chrono::steady_clock::time_point getFrameStartTime() const;

		~AnimatedController() override;
	};

	struct AnimatedBase {
		std::function<void()> onComplete{};
		mutable bool completedNotified = false;
		bool started = false;

		void notifyIfCompleted() const {
			if (!completedNotified && started && onComplete) {
				completedNotified = true;
				onComplete();
			}
		}
	};

	template<class T>
	concept Animatable = requires(T a, T b, float s) {
		a + (b - a) * s;
	};

	template<class T>
	struct Animator {
		static T getValue(const T &from, const T &to, float t) {
			static_assert(Animatable<T>, "Type must support +, -, and *(float) for interpolation, alternatively specialize Animator<T> for custom behavior.");
			return from + ((to - from) * t);
		}
	};

	template<class T>
	struct Animated : AnimatedBase {
		T from{};
		T to = from;
		std::chrono::milliseconds duration = 100ms;
		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
		std::function<float(float)> curve = Curve::easeOutCubic;

		std::shared_ptr<AnimatedController> controller = std::make_shared<AnimatedController>();

		void mount(WidgetStateBase *state) {
			this->controller->element = state->element->weak_from_this();
			this->controller->app = state->element->getApp();
		}

		[[nodiscard]] bool isMounted() const {
			return controller->app != nullptr && !controller->element.expired();
		}

		[[nodiscard]] bool isCompleted() const {
			if (!isMounted()) return false;
			return controller->getFrameStartTime() - startTime >= duration;
		}

		[[nodiscard]] T getValue() const {
			assert(isMounted());
			if (!isMounted()) return from;
			auto now = controller->getFrameStartTime();
			if (now - startTime >= duration) {
				notifyIfCompleted();
				return to;
			}
			auto t = std::chrono::duration<float>(now - startTime).count() / std::chrono::duration<float>(duration).count();
			t = curve(std::clamp(t, 0.f, 1.f));
			return Animator<T>::getValue(from, to, t);
		}

		operator T() const {
			return getValue();
		}

		void setTo(const T &newTo) {
			// If the target value is the same as the current target, do nothing
			if (newTo == to) return;

			from = getValue();
			to = newTo;
			startTime = controller->getFrameStartTime();
			completedNotified = false;
			started = true;

			controller->endTime = startTime + duration;
			controller->run();
		}

		Animated<T> &operator=(const T &newTo) {
			setTo(newTo);
			return *this;
		}
	};

	template<>
	inline Color Animator<Color>::getValue(const Color &from, const Color &to, float t) {
		return from.transition(to, t);
	}

	template<>
	inline vec2 Animator<vec2>::getValue(const vec2 &from, const vec2 &to, float t) {
		return {
			from.x + ((to.x - from.x) * t),
			from.y + ((to.y - from.y) * t),
		};
	}

	template<>
	inline BorderWidth Animator<BorderWidth>::getValue(const BorderWidth &from, const BorderWidth &to, float t) {
		return BorderWidth{
			from.top + ((to.top - from.top) * t),
			from.right + ((to.right - from.right) * t),
			from.bottom + ((to.bottom - from.bottom) * t),
			from.left + ((to.left - from.left) * t),
		};
	}

	template<>
	inline BorderRadius Animator<BorderRadius>::getValue(const BorderRadius &from, const BorderRadius &to, float t) {
		return BorderRadius{
			from.topLeft + ((to.topLeft - from.topLeft) * t),
			from.topRight + ((to.topRight - from.topRight) * t),
			from.bottomRight + ((to.bottomRight - from.bottomRight) * t),
			from.bottomLeft + ((to.bottomLeft - from.bottomLeft) * t),
		};
	}

	template<>
	inline Alignment Animator<Alignment>::getValue(const Alignment &from, const Alignment &to, float t) {
		return Alignment{
			from.horizontal + ((to.horizontal - from.horizontal) * t),
			from.vertical + ((to.vertical - from.vertical) * t),
		};
	}

	template<>
	inline Margin Animator<Margin>::getValue(const Margin &from, const Margin &to, float t) {
		return Margin{
			from.top + ((to.top - from.top) * t),
			from.right + ((to.right - from.right) * t),
			from.bottom + ((to.bottom - from.bottom) * t),
			from.left + ((to.left - from.left) * t),
		};
	}

	template<>
	inline BoxConstraints Animator<BoxConstraints>::getValue(const BoxConstraints &from, const BoxConstraints &to, float t) {
		return BoxConstraints{
			.minWidth = from.minWidth + ((to.minWidth - from.minWidth) * t),
			.maxWidth = from.maxWidth + ((to.maxWidth - from.maxWidth) * t),
			.minHeight = from.minHeight + ((to.minHeight - from.minHeight) * t),
			.maxHeight = from.maxHeight + ((to.maxHeight - from.maxHeight) * t),
			.shrinkWidth = to.shrinkWidth,
			.shrinkHeight = to.shrinkHeight,
		};
	}

	template<>
	inline SizeConstraints Animator<SizeConstraints>::getValue(const SizeConstraints &from, const SizeConstraints &to, float t) {
		return SizeConstraints{
			.minWidth = from.minWidth.value_or(0.f) + ((to.minWidth.value_or(0.f) - from.minWidth.value_or(0.f)) * t),
			.minHeight = from.minHeight.value_or(0.f) + ((to.minHeight.value_or(0.f) - from.minHeight.value_or(0.f)) * t),
			.maxWidth = from.maxWidth.value_or(std::numeric_limits<float>::infinity()) + ((to.maxWidth.value_or(std::numeric_limits<float>::infinity()) - from.maxWidth.value_or(std::numeric_limits<float>::infinity())) * t),
			.maxHeight = from.maxHeight.value_or(std::numeric_limits<float>::infinity()) + ((to.maxHeight.value_or(std::numeric_limits<float>::infinity()) - from.maxHeight.value_or(std::numeric_limits<float>::infinity())) * t),
		};
	}
}// namespace squi::core