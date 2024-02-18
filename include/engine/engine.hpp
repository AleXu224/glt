#pragma once

#include "instance.hpp"
#include <chrono>
#include <functional>

using namespace std::chrono_literals;
namespace Engine {
	struct Vulkan {
		Instance instance;

		uint32_t frameNumber = 0;

		Vulkan();

		Frame &getCurrentFrame();

		void recreateSwapChain();

		bool resized = false;
		std::chrono::time_point<std::chrono::steady_clock> frameStartTime;
		std::chrono::duration<double> deltaTime{0ms};

		void run(const std::function<bool()>& preDraw, const std::function<void()>& drawFunc);

	private:
	};
}// namespace Engine