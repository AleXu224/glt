#pragma once

#include <memory>

#include "image/provider.hpp"
#include "widget.hpp"

namespace squi {
	struct ImageDataImpl;

	struct Image {
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

		// Args
		Widget::Args widget{};
		Fit fit = Fit::none;
		// FIXME: add the ability to provide a sampler shared reference
		ImageProvider image;

		class Impl : public Widget {
			// Data
			Fit fit;
			bool relayoutNextFrame = false;
			std::unique_ptr<ImageDataImpl> data;

		public:
			Impl(const Impl &) = delete;
			Impl(Impl &&) = delete;
			Impl &operator=(const Impl &) = delete;
			Impl &operator=(Impl &&) = delete;
			Impl(const Image &args);
			~Impl() override;

			void onUpdate() override;
			void onLayout(vec2 &maxSize, vec2 &minSize) override;
			void postLayout(vec2 &size) override;
			void postArrange(vec2 &pos) override;
			void onDraw() override;

			[[nodiscard]] ImageDataImpl &getData() {
				return *data;
			}
		};

		operator Child() const {
			return std::make_shared<Impl>(*this);
		}
	};
}// namespace squi