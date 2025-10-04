#pragma once

#include "borderRadius.hpp"
#include "borderWidth.hpp"
#include "color.hpp"
#include "core/animationController.hpp"
#include "core/app.hpp"
#include "core/core.hpp"
#include "core/curve.hpp"
#include <chrono>

namespace squi::core {
	using namespace std::chrono_literals;

	template<class T>
	struct Animated {
		T from{};
		T to = from;
		std::chrono::milliseconds duration = 100ms;
		std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
		std::function<float(float)> curve = Curve::easeOutCubic;

		struct Controller : AnimationController {
			using AnimationController::AnimationController;

			Controller(const Controller &) = delete;
			Controller(Controller &&) noexcept = delete;
			Controller &operator=(const Controller &) = delete;
			Controller &operator=(Controller &&) noexcept = delete;

			ElementPtr element = nullptr;
			App *app = nullptr;

			[[nodiscard]] bool isCompleted() const override {
				if (!app) return false;
				return app->frameStartTime >= endTime;
			}

			void markElementDirty() override {
				if (element) {
					element->markNeedsRebuild();
				}
			}

			void run() {
				if (!app) return;
				app->runningAnimations.insert(this);
				markElementDirty();
			}


			~Controller() override {
				if (app) {
					app->runningAnimations.erase(this);
				}
			}
		};

		std::shared_ptr<Controller> controller = std::make_shared<Controller>();

		void mount(WidgetStateBase *state) {
			this->controller->element = state->element->shared_from_this();
			this->controller->app = state->element->getApp();
		}

		[[nodiscard]] bool isMounted() const {
			return controller->app != nullptr;
		}

		[[nodiscard]] bool isCompleted() const {
			if (!isMounted()) return false;
			return controller->app->frameStartTime - startTime >= duration;
		}

		[[nodiscard]] T getValue() const {
			static_assert(std::is_arithmetic_v<T>, "Animation only supports arithmetic types");
			assert(isMounted());
			if (!isMounted()) return from;
			auto now = controller->app->frameStartTime;
			if (now - startTime >= duration) return to;
			auto t = std::chrono::duration<float>(now - startTime).count() / std::chrono::duration<float>(duration).count();
			t = curve(t);
			return from + ((to - from) * t);
		}

		operator T() const {
			return getValue();
		}

		void setTo(const T &newTo) {
			// If the target value is the same as the current target, do nothing
			if (newTo == to) return;

			from = getValue();
			to = newTo;
			startTime = controller->app->frameStartTime;

			controller->endTime = startTime + duration;
			controller->run();
		}

		Animated<T> &operator=(const T &newTo) {
			setTo(newTo);
			return *this;
		}
	};

	template<>
	inline Color Animated<Color>::getValue() const {
		if (!isMounted()) return from;
		auto now = controller->app->frameStartTime;
		if (now - startTime >= duration) return to;
		auto t = std::chrono::duration<float>(now - startTime).count() / std::chrono::duration<float>(duration).count();
		t = curve(t);
		return from.transition(to, t);
	}

	template<>
	inline BorderWidth Animated<BorderWidth>::getValue() const {
		if (!isMounted()) return from;
		auto now = controller->app->frameStartTime;
		if (now - startTime >= duration) return to;
		auto t = std::chrono::duration<float>(now - startTime).count() / std::chrono::duration<float>(duration).count();
		t = curve(t);
		return BorderWidth{
			from.top + ((to.top - from.top) * t),
			from.right + ((to.right - from.right) * t),
			from.bottom + ((to.bottom - from.bottom) * t),
			from.left + ((to.left - from.left) * t),
		};
	}

	template<>
	inline BorderRadius Animated<BorderRadius>::getValue() const {
		if (!isMounted()) return from;
		auto now = controller->app->frameStartTime;
		if (now - startTime >= duration) return to;
		auto t = std::chrono::duration<float>(now - startTime).count() / std::chrono::duration<float>(duration).count();
		t = curve(t);
		return BorderRadius{
			from.topLeft + ((to.topLeft - from.topLeft) * t),
			from.topRight + ((to.topRight - from.topRight) * t),
			from.bottomRight + ((to.bottomRight - from.bottomRight) * t),
			from.bottomLeft + ((to.bottomLeft - from.bottomLeft) * t),
		};
	}

	template<>
	inline Alignment Animated<Alignment>::getValue() const {
		if (!isMounted()) return from;
		auto now = controller->app->frameStartTime;
		if (now - startTime >= duration) return to;
		auto t = std::chrono::duration<float>(now - startTime).count() / std::chrono::duration<float>(duration).count();
		t = curve(t);
		return Alignment{
			from.horizontal + ((to.horizontal - from.horizontal) * t),
			from.vertical + ((to.vertical - from.vertical) * t),
		};
	}

	template<>
	inline Margin Animated<Margin>::getValue() const {
		if (!isMounted()) return from;
		auto now = controller->app->frameStartTime;
		if (now - startTime >= duration) return to;
		auto t = std::chrono::duration<float>(now - startTime).count() / std::chrono::duration<float>(duration).count();
		t = curve(t);
		return Margin{
			from.top + ((to.top - from.top) * t),
			from.right + ((to.right - from.right) * t),
			from.bottom + ((to.bottom - from.bottom) * t),
			from.left + ((to.left - from.left) * t),
		};
	}

	template<>
	inline BoxConstraints Animated<BoxConstraints>::getValue() const {
		if (!isMounted()) return from;
		auto now = controller->app->frameStartTime;
		if (now - startTime >= duration) return to;
		auto t = std::chrono::duration<float>(now - startTime).count() / std::chrono::duration<float>(duration).count();
		t = curve(t);
		return BoxConstraints{
			.minWidth = from.minWidth + ((to.minWidth - from.minWidth) * t),
			.maxWidth = from.maxWidth + ((to.maxWidth - from.maxWidth) * t),
			.minHeight = from.minHeight + ((to.minHeight - from.minHeight) * t),
			.maxHeight = from.maxHeight + ((to.maxHeight - from.maxHeight) * t),
			.shrinkWidth = to.shrinkWidth,
			.shrinkHeight = to.shrinkHeight,
		};
	}
}// namespace squi::core