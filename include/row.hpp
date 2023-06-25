#ifndef SQUI_ROW_HPP
#define SQUI_ROW_HPP

#include "widget.hpp"
#include <memory>

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

            void layoutChildren(vec2 &maxSize, vec2 &minSize) final;
            void arrangeChildren(vec2 &pos) final;

            float getMinWidth() final;

            void drawChildren() final;
		};

		operator Child() const {
			return {std::make_unique<Impl>(*this)};
		}
	};
};// namespace squi

#endif