#ifndef SQUI_ANIMATEDFLOAT_HPP
#define SQUI_ANIMATEDFLOAT_HPP

#include "chrono"
#include "functional"

using namespace std::chrono_literals;

namespace squi {
	class AnimatedFloat {
		struct Animation {
			float initialValue = 0;
			float finalValue = 0;
			std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
			std::chrono::duration<double> duration = 0s;
			std::function<double(double)> easingFunction = [](double t) { return t; };
			mutable bool finished = false;
		};
		mutable Animation animation;

	public:
		AnimatedFloat() = default;
		AnimatedFloat(float value)
			: animation(Animation{
				  .initialValue = value,
				  .finalValue = value,
			  }) {}

		void animateTo(
			float newValue,
			std::chrono::duration<double> duration,
			std::function<float(float)> easingFunction = [](float t) { return t; }) const;
		void cancelAnimation();

		operator float() const;

		AnimatedFloat& operator= (float value) {
			animation.finalValue = value;
			animation.finished = true;

			return *this;
		}

		[[nodiscard]] float operator+(const float& other) const;
		[[nodiscard]] float operator-(const float& other) const;
		[[nodiscard]] float operator*(const float& other) const;
		[[nodiscard]] float operator/(const float& other) const;

		AnimatedFloat& operator+=(const float& other);
		AnimatedFloat& operator-=(const float& other);
		AnimatedFloat& operator*=(const float& other);
		AnimatedFloat& operator/=(const float& other);
	};
}// namespace squi

#endif//SQUI_ANIMATEDFLOAT_HPP