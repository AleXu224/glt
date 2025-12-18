#pragma once

#include "color.hpp"
#include "core/core.hpp"
#include "widgets/box.hpp"

namespace squi {
	struct Button : StatefulWidget {
		enum class ButtonStatus : uint8_t {
			resting,
			hovered,
			active,
			disabled,
		};

		struct Style {
			Color backgroundColor{};
			Color borderColor{};
			Box::BorderPosition borderPosition = Box::BorderPosition::inset;
			BorderRadius borderRadius{4.0f};
			BorderWidth borderWidth{1.0f};
			Color textColor{0x000000FF};
			float textSize{14.0f};
			Alignment alignment = Alignment::Center;
		};

		struct Theme {
			Style resting;
			Style hovered;
			Style active;
			Style disabled;

			static Theme Accent();
			static Theme Standard();
			static Theme Subtle();

			auto &&fromStatus(this auto &&self, ButtonStatus status) {
				switch (status) {
					case ButtonStatus::resting:
						return self.resting;
					case ButtonStatus::hovered:
						return self.hovered;
					case ButtonStatus::active:
						return self.active;
					case ButtonStatus::disabled:
						return self.disabled;
				}
				throw std::runtime_error("Invalid button status");
			}
		};

		// Args
		Key key;
		Args widget{};
		Theme theme = Theme::Standard();
		bool disabled;
		std::function<void(ButtonStatus)> onStatusChange{};
		std::function<void()> onClick{};
		std::variant<std::string, Child> child = "Button";

		struct State : WidgetState<Button> {
			ButtonStatus status = ButtonStatus::resting;
			bool isHovered = false;
			bool isActive = false;

			void updateStatus();

			[[nodiscard]] Child getContent(const Style &style) const;

			[[nodiscard]] Args getArgs() const;

			void initState() override {
				updateStatus();
			}

			void widgetUpdated() override {
				updateStatus();
			}

			Child build(const Element &) override;
		};
	};
}// namespace squi