#pragma once

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

			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) override;
			void arrangeChildren(vec2 &pos) final;

			void drawChildren() final;
		};

		operator Child() const {
			return std::make_unique<Impl>(*this);
		}
	};
};// namespace squi