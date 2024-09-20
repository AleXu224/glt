#pragma once
#include "widgetArgs.hpp"
#include <functional>
#include <optional>

namespace squi {
	struct Checkbox {
		// Args
		Args widget{};
		std::optional<std::string_view> text{};
		std::optional<std::reference_wrapper<bool>> value{};
		std::function<void(bool)> onChange{};

		struct Storage {
			// Data
			std::optional<std::reference_wrapper<bool>> value{};
			bool internalValue{false};
		};

		operator Child() const;
	};
}// namespace squi