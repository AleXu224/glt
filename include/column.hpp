#pragma once

#include "widget.hpp"
#include <memory>

namespace squi {
	struct Column {
		// Helpers
		enum class Alignment {
			left,
			center,
			right,
		};

		// Args
		Widget::Args widget{};
		Alignment alignment{Alignment::left};
		float spacing{0.0f};
		Children children{};

		class Impl : public Widget {
			// Data
			Alignment alignment;
			float spacing;

		public:
			explicit Impl(const Column &args);

			vec2 layoutChildren(vec2 maxSize, vec2 minSize, ShouldShrink shouldShrink, bool final) final;
			void arrangeChildren(vec2 &pos) final;

			void drawChildren() final;
		};

		operator Child() const {
			return std::make_unique<Impl>(*this);
		}
	};
}// namespace squi