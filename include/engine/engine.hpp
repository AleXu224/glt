#pragma once

#include "instance.hpp"
#include <functional>

namespace Engine {
	struct Vulkan {
		Instance instance;

		uint32_t frameNumber = 0;

		Vulkan();

		Frame &getCurrentFrame();

		void recreateSwapChain();

		std::function<void()> drawFunc{};

		bool resized = false;

		void run(std::function<void()>);
	private:
	};
}// namespace Engine