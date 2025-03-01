#include "inputQueue.hpp"

#include "chrono"
#include "utils.hpp"


using namespace squi;

void squi::InputQueue::push(const InputTypes &item) {
	std::scoped_lock lock{inputMtx};
	bool handled = false;
	std::visit(
		utils::overloaded{
			[&](const CursorPosInput &input) {
				if (!inputQueue.empty() && std::holds_alternative<CursorPosInput>(inputQueue.back())) {
					auto &entry = std::get<CursorPosInput>(inputQueue.back());
					entry.xPos = input.xPos;
					entry.yPos = input.yPos;
					handled = true;
				}
			},
			[](auto &&) {},
		},
		item
	);
	if (!handled)
		inputQueue.push(item);

	if (!inputNotified) {
		inputPromise.set_value();
		inputNotified = true;
	}
}

std::optional<squi::InputTypes> squi::InputQueue::pop() {
	std::scoped_lock lock{inputMtx};
	if (inputQueue.empty())
		return {};

	auto item = inputQueue.front();
	inputQueue.pop();
	if (inputQueue.empty())
		resetInputPromise();
	return item;
}

bool squi::InputQueue::waitForInput() {
	{
		std::scoped_lock lock{inputMtx};
		if (!inputQueue.empty()) return true;
	}

	using namespace std::chrono_literals;

	inputFuture.wait_for(100ms);

	{
		std::scoped_lock lock{inputMtx};
		resetInputPromise();
		if (!inputQueue.empty()) return true;
	}

	return false;
}

void squi::InputQueue::resetInputPromise() {
	inputPromise = std::promise<void>{};
	inputFuture = inputPromise.get_future();
	inputNotified = false;
}
