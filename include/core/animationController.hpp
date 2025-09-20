#pragma once

#include <chrono>

namespace squi::core {
	struct AnimationController {
		std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();

		AnimationController() = default;

		AnimationController(const AnimationController &) = delete;
		AnimationController(AnimationController &&) = delete;
		AnimationController &operator=(const AnimationController &) = delete;
		AnimationController &operator=(AnimationController &&) = delete;
		virtual ~AnimationController() = default;

		[[nodiscard]] virtual bool isCompleted() const = 0;

		virtual void markElementDirty() = 0;
	};
}// namespace squi::core