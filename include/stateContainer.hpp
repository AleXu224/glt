#pragma once

#include "any"
#include "string"
#include "vector"
#include <stdexcept>
#include <unordered_map>


namespace squi {
	struct StateContainer {
		std::string name;
		std::any value;

		template<class T>
		StateContainer(T &&state) : name(typeid(T).name()), value(std::forward<T>(state)) {}
		template<class T>
		StateContainer(std::string_view name, T &&state) : name(name), value(std::forward<T>(state)) {}
	};

	struct CustomState {
		template<class T>
		[[nodiscard]] T &get() {
			const auto *name = typeid(T).name();
			if (auto iterator = states.find(name); iterator != states.end()) {
				return std::any_cast<T &>(iterator->second);
			}
			throw std::runtime_error("Cannot get the state from the widget");
		}

		template<class T>
		[[nodiscard]] T &get(const std::string &name) {
			if (auto iterator = states.find(name); iterator != states.end()) {
				return std::any_cast<T &>(iterator->second);
			}
			throw std::runtime_error("Cannot get the state from the widget");
		}

		template<class T>
		void add(const std::string &name, T &&state) {
			if (auto iterator = states.find(name); iterator == states.end()) {
				states.insert(std::pair(name, std::forward<T>(state)));
				return;
			}
			throw std::runtime_error("State of this type already in widget");
		}

		void add(const StateContainer &state);

		void add(const std::vector<StateContainer> &states);

	private:
		std::unordered_map<std::string, std::any> states{};
	};
}// namespace squi