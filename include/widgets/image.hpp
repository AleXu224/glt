#pragma once

#include "core/core.hpp"
#include "image/provider.hpp"

namespace squi {
	struct ImageDataImpl;
	struct Image : core::RenderObjectWidget {
		enum class Fit : uint8_t {
			// The image will be displayed at its original size.
			none,
			// The image will be resized and stretched to fill the widget.
			fill,
			// The image will be resized to fill the widget while maintaining the aspect ratio.
			cover,
			// The image will be resized to completely fit inside the widget while maintaining the aspect ratio.
			contain,
		};
		Key key;
		Args widget{};
		Fit fit = Fit::none;
		ImageProvider image;

		struct ImageRenderObject : core::SingleChildRenderObject {
			Fit fit = Fit::none;
			ImageProvider imageProvider;
			std::unique_ptr<ImageDataImpl> data;

			ImageRenderObject();

			void init() override;
			vec2 calculateContentSize(BoxConstraints constraints, bool final) override;
			void drawSelf() override;
		};

		static std::shared_ptr<RenderObject> createRenderObject();

		void updateRenderObject(RenderObject *renderObject) const;

		[[nodiscard]] Args getArgs() const {
			Args widget = this->widget;
			widget.width = widget.width.value_or(Size::Wrap);
			widget.height = widget.height.value_or(Size::Wrap);
			return widget;
		}
	};
}// namespace squi