#ifndef SQUI_ROW_HPP
#define SQUI_ROW_HPP

#include "widget.hpp"

namespace squi {
	struct Row {
        // Helpers
        enum class Alignment {
            top,
            center,
            bottom,
        };

		// Args
		Widget::Args widget{};
        Alignment alignment{Alignment::top};
        float spacing{0.0f};
        Children children{};

		class Impl : public Widget {
			// Data
            Alignment alignment;
            float spacing;

		public:
			Impl(const Row &args);

            void matchChildSizeBehavior(bool horizontalHint, bool verticalHint) override;

            void onUpdate() override;

            void onDraw() override;
		};

		operator std::shared_ptr<Widget>() const {
			return std::make_shared<Impl>(*this);
		}

		operator Child() const {
			return Child(std::make_shared<Impl>(*this));
		}
	};
};// namespace squi

#endif