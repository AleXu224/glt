#pragma once

#include "mutex"
#include <future>
#include <queue>
#include <variant>

namespace squi {
	struct CursorPosInput {
		float xPos;
		float yPos;
	};

	struct CodepointInput {
		char character;
	};

	struct ScrollInput {
		float xOffset;
		float yOffset;
	};

	struct KeyInput {
		int key;
		int action;
		int mods;
	};

	struct MouseInput {
		int button;
		int action;
		int mods;
	};

	struct CursorEntered {
		bool entered;
	};

	struct StateChange {};

	using InputTypes = std::variant<CursorPosInput, CodepointInput, ScrollInput, KeyInput, MouseInput, CursorEntered, StateChange>;

	struct InputQueue {
		void push(const InputTypes &item);

		std::optional<InputTypes> pop();

		bool waitForInput();

	private:
		std::mutex inputMtx{};
		std::queue<InputTypes> inputQueue{};

		bool promiseRetrieved = false;
		std::promise<void> inputPromise;
	};
}// namespace squi