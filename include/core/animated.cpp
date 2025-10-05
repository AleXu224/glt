#include "core/animated.hpp"

#include "app.hpp"

namespace squi::core {

	[[nodiscard]] bool AnimatedController::isCompleted() const {
		if (!app) return false;
		return app->frameStartTime >= endTime;
	}

	void AnimatedController::run() {
		if (!app) return;
		app->runningAnimations.insert(this);
		markElementDirty();
	}

	[[nodiscard]] std::chrono::steady_clock::time_point AnimatedController::getFrameStartTime() const {
		return app->frameStartTime;
	}

	AnimatedController::~AnimatedController() {
		if (app) {
			app->runningAnimations.erase(this);
		}
	}
}