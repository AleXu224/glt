#pragma once

#include "instance.hpp"
#include <chrono>
#include <functional>

using namespace std::chrono_literals;
namespace Engine {
	struct Runner {
		Instance instance;

		uint32_t frameNumber = 0;

		Runner(WindowOptions options = {});

		Frame &getCurrentFrame();

		void recreateSwapChain();

		bool resized = false;
		bool outdatedFramebuffer = false;
		std::mutex swapChainMtx{};
		std::chrono::time_point<std::chrono::steady_clock> frameStartTime;
		std::chrono::duration<double> deltaTime{0ms};

		void run(const std::function<bool()> &preDraw, const std::function<void()> &drawFunc, const std::function<void()> &cleanupFunc);

		void draw();

		std::function<bool()> preDraw{};
		std::function<void()> drawFunc{};
		std::function<void()> cleanupFunc{};
	};
}// namespace Engine