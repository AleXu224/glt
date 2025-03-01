#include "inputQueue.hpp"

void squi::InputQueue::push(const InputTypes &item) {
	std::scoped_lock lock{inputMtx};
	inputQueue.push(item);
	if (!promiseRetrieved)
		inputPromise.set_value();
}

std::optional<squi::InputTypes> squi::InputQueue::pop() {
	std::scoped_lock lock{inputMtx};
	if (inputQueue.empty())
		return {};

	auto item = inputQueue.front();
	inputQueue.pop();
	if (inputQueue.empty()) {
		inputPromise = std::promise<void>{};
		promiseRetrieved = true;
	}
	return item;
}

bool squi::InputQueue::waitForInput() {
	{
		std::scoped_lock lock{inputMtx};
		if (!inputQueue.empty()) return true;
	}

	using namespace std::chrono_literals;

	inputPromise.get_future().wait_for(100ms);

	{
		std::scoped_lock lock{inputMtx};
		inputPromise = std::promise<void>{};
		promiseRetrieved = true;
		if (!inputQueue.empty()) return true;
	}

	return false;
}
