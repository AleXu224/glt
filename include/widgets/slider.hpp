#pragma once

#include "core/core.hpp"

namespace squi {
	struct Slider : StatefulWidget {
		struct TickInterval {
			float interval;
			float offset = 0.f;
		};
		struct TickCount {
			uint32_t count;
		};

		// Args
		Key key;
		Args widget{};
		float minValue = 0.f;
		float maxValue = 100.f;
		float value = 0.f;
		using TicksVariant = std::variant<std::monostate, TickCount, TickInterval, std::vector<float>>;
		TicksVariant ticks = std::monostate{};
		std::function<void(float)> onChange;

		struct State : WidgetState<Slider> {
			std::vector<float> computeTicks() const;
			Key handleKey = GlobalKey{};
			Key tooltipKey = GlobalKey{};
			bool focused = false;
			bool handleHovered = false;

			float getHandleInnerSize() const;
			void createOrUpdateTooltip();

			void widgetUpdated() override {
				if (focused) {
					createOrUpdateTooltip();
				}
			}

			Child build(const Element &element) override;
		};
	};
}// namespace squi