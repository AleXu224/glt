#pragma once
#include "chrono"
#include "functional"
using namespace std::chrono_literals;

namespace squi {
	struct Animation {
		struct Ease {
			static float linear(float t) {
				return t;
			}

			static float easeInCubic(float t) {
				return t * t * t;
			}

			static float easeOutCubic(float t) {
				return 1.0f - easeInCubic(1.0f - t);
			}

			static float easeInOutCubic(float t) {
				return t < 0.5f ? easeInCubic(t * 2.0f) / 2.0f : 1.0f - easeInCubic((1.0f - t) * 2.0f) / 2.0f;
			}
		};

		const std::chrono::milliseconds duration;
		const std::chrono::steady_clock::time_point startTime{std::chrono::steady_clock::now()};
		const std::function<float(float)> easeFunction{Ease::linear};
		const std::function<void(float)> onUpdate;
		const std::function<void()> onFinish{};

		[[nodiscard]] float getProgress() const;
		void update();
	};
}// namespace squi