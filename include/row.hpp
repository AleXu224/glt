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

            void onLayout(vec2 &maxSize, vec2 &minSize) override;
            void onArrange(vec2 &pos) override;

            void onDraw() override;
		};

		operator Child() const {
			return Child(std::make_shared<Impl>(*this));
		}
	};
};// namespace squi

#endif