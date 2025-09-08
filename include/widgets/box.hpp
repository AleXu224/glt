#pragma once

#include "borderRadius.hpp"
#include "borderWidth.hpp"
#include "color.hpp"
#include "core/core.hpp"


namespace squi {
	struct BoxData;
	struct Box : core::RenderObjectWidget {
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
		Child child;

		struct BoxRenderObject : core::SingleChildRenderObject {
			std::unique_ptr<BoxData> data;

			bool shouldClipContent = true;

			BoxRenderObject(Args args = Args{});

			void init() override;
			void drawSelf() override;
		};

		std::shared_ptr<RenderObject> createRenderObject() const;

		void updateRenderObject(RenderObject *renderObject) const;
	};
}// namespace squi