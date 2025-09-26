#pragma once

#include "core/core.hpp"

namespace squi {
	struct Wrapper : RenderObjectWidget {
		Key key;
		std::function<void(BoxConstraints &, bool, RenderObject &)> beforeLayout;
		std::function<void(RenderObject &)> afterLayout;

		std::function<void(Rect &, RenderObject &)> beforePosition;
		std::function<void(RenderObject &)> afterPosition;

		std::function<void(RenderObject &)> beforeDraw;
		std::function<void(RenderObject &)> afterDraw;

		Child child;

		struct Element : SingleChildRenderObjectElement {
			using SingleChildRenderObjectElement::SingleChildRenderObjectElement;

			Child build() override {
				if (auto offsetWidget = std::static_pointer_cast<Wrapper>(widget)) {
					return offsetWidget->child;
				}
				return {};
			};
		};

		struct WrapperRenderObject : SingleChildRenderObject {
			vec2 calculateContentSize(BoxConstraints constraints, bool final) override {
				auto *widget = this->getWidgetAs<Wrapper>();
				if (widget->beforeLayout) {
					widget->beforeLayout(constraints, final, *this);
				}
				auto size = SingleChildRenderObject::calculateContentSize(constraints, final);
				if (widget->afterLayout) {
					widget->afterLayout(*this);
				}
				return size;
			}

			void positionContentAt(const Rect &newBounds) override {
				auto *widget = this->getWidgetAs<Wrapper>();
				Rect bounds = newBounds;
				if (widget->beforePosition) {
					widget->beforePosition(bounds, *this);
				}
				SingleChildRenderObject::positionContentAt(bounds);
				if (widget->afterPosition) {
					widget->afterPosition(*this);
				}
			}

			void drawContent() override {
				auto *widget = this->getWidgetAs<Wrapper>();
				if (widget->beforeDraw) {
					widget->beforeDraw(*this);
				}
				SingleChildRenderObject::drawContent();
				if (widget->afterDraw) {
					widget->afterDraw(*this);
				}
			}
		};

		static std::shared_ptr<RenderObject> createRenderObject() {
			return std::make_shared<WrapperRenderObject>();
		}

		void updateRenderObject(RenderObject *renderObject) const {}

		static Args getArgs() {
			return {
				.width = Size::Wrap,
				.height = Size::Wrap,
			};
		}
	};
}// namespace squi