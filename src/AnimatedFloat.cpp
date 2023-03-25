#include "AnimatedFloat.hpp"

#include <utility>
#include "algorithm"

using namespace squi;

void AnimatedFloat::animateTo(float newValue, std::chrono::duration<double> duration, std::function<float(float)> easingFunction) const {
	if (animation.finalValue == newValue)
		return;

	animation = Animation{
		.initialValue = operator float(),
		.finalValue = newValue,
		.startTime = std::chrono::steady_clock::now(),
		.duration = duration,
		.easingFunction = std::move(easingFunction),
	};
}

void AnimatedFloat::cancelAnimation() {
	animation.duration = 0s;
}

AnimatedFloat::operator float() const {
	if (animation.finished || animation.duration == 0s) return animation.finalValue;

	const auto timeSinceStart = std::chrono::steady_clock::now() - animation.startTime;
	const auto progress = animation.easingFunction(timeSinceStart / animation.duration);
	if (progress >= 1) {
		animation.finished = true;
		return animation.finalValue;
	}
	return animation.initialValue + static_cast<float>(progress) * (animation.finalValue - animation.initialValue);
}

float AnimatedFloat::operator+(const float &other) const {
	return operator float() + other;
}

float AnimatedFloat::operator-(const float &other) const {
	return operator float() - other;
}

float AnimatedFloat::operator*(const float &other) const {
	return operator float() * other;
}

float AnimatedFloat::operator/(const float &other) const {
	return operator float() / other;
}

AnimatedFloat &AnimatedFloat::operator+=(const float &other) {
	animation.finalValue += other;
	animation.finished = true;
	return *this;
}

AnimatedFloat &AnimatedFloat::operator-=(const float &other) {
	animation.finalValue -= other;
	animation.finished = true;
	return *this;
}

AnimatedFloat &AnimatedFloat::operator*=(const float &other) {
	animation.finalValue *= other;
	animation.finished = true;
	return *this;
}

AnimatedFloat &AnimatedFloat::operator/=(const float &other) {
	animation.finalValue /= other;
	animation.finished = true;
	return *this;
}