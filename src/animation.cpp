#include "animation.hpp"
#include <algorithm>

using namespace squi;

float squi::Animation::getProgress() const {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    return std::clamp(static_cast<float>(elapsed.count()) / static_cast<float>(duration.count()), 0.0f, 1.0f);
}

void Animation::update() {
    const auto progress = getProgress();
    onUpdate(easeFunction(progress));
    if (progress >= 1.0f) {
        onFinish();
    }
}