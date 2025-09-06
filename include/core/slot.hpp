#pragma once

#include "compare"

namespace squi::core {
	struct Slot {
		virtual ~Slot() = default;

		virtual std::strong_ordering operator<=>(const Slot &other) const = 0;
	};

	struct IndexSlot : Slot {
		size_t index;

		explicit IndexSlot(size_t index) : index(index) {}

		std::strong_ordering operator<=>(const Slot &other) const override {
			if (auto *otherIndexSlot = dynamic_cast<const IndexSlot *>(&other)) {
				return index <=> otherIndexSlot->index;
			}
			return std::strong_ordering::less;
		}
	};
}// namespace squi::core