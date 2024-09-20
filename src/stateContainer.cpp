#include "stateContainer.hpp"

using namespace squi;
void squi::CustomState::add(const StateContainer &state) {
	if (auto iterator = states.find(state.name); iterator == states.end()) {
		states.insert(std::pair(state.name, state.value));
		return;
	}
	throw std::runtime_error("State of this type already in widget");
}

void squi::CustomState::add(const std::vector<StateContainer> &states) {
	for (const auto &state: states) {
		add(state);
	}
}
