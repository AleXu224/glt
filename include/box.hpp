#pragma once

#include "borderRadius.hpp"
#include "borderWidth.hpp"
#include "color.hpp"
#include "widget.hpp"
#include <glm/fwd.hpp>
#include <memory>


namespace squi {
	struct BoxData;
	struct Box {
		enum class BorderPosition : uint8_t {
			inset,
			outset,
		};

		Args widget{};
		Color color{0xFFFFFFFF};
		Color borderColor{0x000000FF};
		BorderWidth borderWidth{0.0f};
		BorderRadius borderRadius{0.0f};
		BorderPosition borderPosition{BorderPosition::inset};
		bool shouldClipContent = true;
		Child child{};
		class Impl : public Widget {
			// pimpl for the pipeline since it is an expensive header
			std::unique_ptr<BoxData> data;

			BorderPosition borderPosition;
			bool shouldClipContent;

		public:
			Impl(const Impl &) = delete;
			Impl(Impl &&) = delete;
			Impl &operator=(const Impl &) = delete;
			Impl &operator=(Impl &&) = delete;
			explicit Impl(const Box &args);
			~Impl() override;

			void onDraw() final;
			void drawChildren() final;

			void postLayout(vec2 &size) final;
			void postArrange(vec2 &pos) final;

			void setColor(const Color &color);
			void setBorderColor(const Color &color);
			void setBorderWidth(glm::vec4 width);
			void setBorderRadius(glm::vec4 radius);

			[[nodiscard]] Color getColor() const;
			[[nodiscard]] Color getBorderColor() const;
			[[nodiscard]] glm::vec4 getBorderWidth() const;
			[[nodiscard]] glm::vec4 getBorderRadius() const;
			[[nodiscard]] BoxData &getData();
		};

		operator Child() {
			return {std::make_shared<Impl>(*this)};
		}
	};
}// namespace squi
