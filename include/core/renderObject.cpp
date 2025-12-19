#include "renderObject.hpp"

#include "algorithm"
#include "core/app.hpp"
#include "utils.hpp"


namespace squi::core {
	// Render Object
	App *RenderObject::getApp() const {
		assert(app != nullptr);
		return app;
	}

	vec2 RenderObject::calculateSize(BoxConstraints extConstraints, bool final) {
		const auto &intConstraints = sizeConstraints;

		// Make it so that the constraints always allow the widget to be at least its margin + padding size
		extConstraints.extendToFitMarginPadding(margin, padding);
		// Margin is considered to be outside the widget, so we need to remove it from the constraints
		extConstraints.offsetMaxSize(-margin.getSizeOffset());
		// Get the minimum between the external constraints and the internal constraints
		extConstraints.clampMaxSizeWith(intConstraints);

		auto marginOffset = margin.getSizeOffset();
		auto paddingOffset = padding.getSizeOffset();

		// Handle the size mode constraints
		std::visit(
			utils::overloaded{
				[&](const float &val) {
					extConstraints.minWidth = paddingOffset.x;
					extConstraints.maxWidth = std::clamp(val, extConstraints.minWidth, extConstraints.maxWidth);
					extConstraints.shrinkWidth = false;
				},
				[&](const Size &size) {
					if (size == Size::Shrink) {
						extConstraints.shrinkWidth = true;
						extConstraints.minWidth = paddingOffset.x;
					} else {
						extConstraints.minWidth = std::clamp(extConstraints.minWidth - marginOffset.x, paddingOffset.x, extConstraints.maxWidth);
					}
				},
			},
			width
		);

		std::visit(
			utils::overloaded{
				[&](const float &val) {
					extConstraints.minHeight = paddingOffset.y;
					extConstraints.maxHeight = std::clamp(val, extConstraints.minHeight, extConstraints.maxHeight);
					extConstraints.shrinkHeight = false;
				},
				[&](const Size &size) {
					if (size == Size::Shrink) {
						extConstraints.shrinkHeight = true;
						extConstraints.minHeight = paddingOffset.y;
					} else {
						extConstraints.minHeight = std::clamp(extConstraints.minHeight - marginOffset.y, paddingOffset.y, extConstraints.maxHeight);
					}
				},
			},
			height
		);

		// Handle the min size constraints
		extConstraints.minWidth = std::clamp(intConstraints.minWidth, extConstraints.minWidth, extConstraints.maxWidth);
		extConstraints.minHeight = std::clamp(intConstraints.minHeight, extConstraints.minHeight, extConstraints.maxHeight);

		auto contentConstraints = extConstraints.withoutPadding(padding);
		if (std::holds_alternative<float>(width) || std::get<Size>(width) != Size::Wrap) {
			contentConstraints.minWidth = 0.f;
		}
		if (std::holds_alternative<float>(height) || std::get<Size>(height) != Size::Wrap) {
			contentConstraints.minHeight = 0.f;
		}

		auto contentSize = calculateContentSize(contentConstraints, final);

		bool needsContentSizeRecalc = false;
		if (extConstraints.shrinkWidth && contentSize.x + paddingOffset.x < extConstraints.minWidth) {
			needsContentSizeRecalc = true;
			contentConstraints.maxWidth = extConstraints.minWidth - paddingOffset.x;
			contentConstraints.shrinkWidth = false;
		}
		if (extConstraints.shrinkHeight && contentSize.y + paddingOffset.y < extConstraints.minHeight) {
			needsContentSizeRecalc = true;
			contentConstraints.maxHeight = extConstraints.minHeight - paddingOffset.y;
			contentConstraints.shrinkHeight = false;
		}
		if (needsContentSizeRecalc) {
			contentSize = calculateContentSize(contentConstraints, final);
		}

		extConstraints.minWidth = std::clamp(contentSize.x + paddingOffset.x, extConstraints.minWidth, extConstraints.maxWidth);
		extConstraints.minHeight = std::clamp(contentSize.y + paddingOffset.y, extConstraints.minHeight, extConstraints.maxHeight);

		std::visit(
			utils::overloaded{
				[&](const float &val) {
					size.x = std::clamp(val, extConstraints.minWidth, extConstraints.maxWidth);
				},
				[&](const Size &val) {
					switch (val) {
						case Size::Expand: {
							if (extConstraints.shrinkWidth)
								size.x = extConstraints.minWidth;
							else
								size.x = extConstraints.maxWidth;
							break;
						}
						case Size::Wrap:
						case Size::Shrink: {
							size.x = extConstraints.minWidth;
							break;
						}
					}
				},
			},
			width
		);

		std::visit(
			utils::overloaded{
				[&](const float &val) {
					size.y = std::clamp(val, extConstraints.minHeight, extConstraints.maxHeight);
				},
				[&](const Size &val) {
					switch (val) {
						case Size::Expand: {
							if (extConstraints.shrinkHeight)
								size.y = extConstraints.minHeight;
							else
								size.y = extConstraints.maxHeight;
							break;
						}
						case Size::Wrap:
						case Size::Shrink: {
							size.y = extConstraints.minHeight;
							break;
						}
					}
				},
			},
			height
		);

		if (final) {
			// Can do stuff here that should only be done once the size is final
			afterSizeCalculated();
		}

		return size + marginOffset;
	}

	vec2 RenderObject::calculateContentSize(BoxConstraints, bool) {
		return {};
	}

	void RenderObject::positionAt(const Rect &newBounds) {
		pos = newBounds.posFromAlignment(alignment.value_or(Alignment::TopLeft), getLayoutRect()) + margin.getPositionOffset();

		auto wrapWidth = (std::holds_alternative<Size>(width) && std::get<Size>(width) == Size::Wrap);
		auto wrapHeight = (std::holds_alternative<Size>(height) && std::get<Size>(height) == Size::Wrap);

		auto contentBounds = getContentRect();
		auto offset = padding.getPositionOffset() + margin.getPositionOffset();

		// If the widget is wrapping then allow the content to be positioned relative to the parent bounds
		if (wrapWidth && !alignment.has_value()) {
			contentBounds.left = newBounds.left + offset.x;
			contentBounds.right = newBounds.right - offset.x;
		}

		if (wrapHeight && !alignment.has_value()) {
			contentBounds.top = newBounds.top + offset.y;
			contentBounds.bottom = newBounds.bottom - offset.y;
		}

		positionContentAt(contentBounds);

		if (!wrapWidth && !wrapHeight) return;
		auto children = getChildren();
		if (children.empty()) return;
		auto &firstChild = children.front();
		if (wrapWidth && !alignment.has_value()) {
			pos.x = firstChild->getLayoutRect().left - padding.left;
		}

		if (wrapHeight && !alignment.has_value()) {
			pos.y = firstChild->getLayoutRect().top - padding.top;
		}
	}

	void RenderObject::draw() {
		drawSelf();
		drawContent();
	}

	Rect RenderObject::getRect() const {
		return Rect::fromPosSize(pos, size);
	}

	Rect RenderObject::getContentRect() const {
		return Rect::fromPosSize(
			pos + padding.getPositionOffset(),
			size - padding.getSizeOffset()
		);
	}

	Rect RenderObject::getLayoutRect() const {
		return Rect::fromPosSize(
			pos - margin.getPositionOffset(),
			size + margin.getSizeOffset()
		);
	}

	Rect RenderObject::getHitcheckRect() const {
		return getRect();
	}

	void RenderObject::initRenderObject() {
		auto &widget = this->element->widget;
		if (auto renderObjectWidget = std::static_pointer_cast<RenderObjectWidget>(widget)) {
			this->updateWidgetArgs(renderObjectWidget->_getWidgetArgs());
		}

		init();
	}

	void RenderObject::updateWidgetArgs(const Args &args) {
		auto *app = this->getApp();

		if (width != args.width.value_or(Size::Expand)) {
			width = args.width.value_or(Size::Expand);
			app->needsRelayout = true;
		}
		if (height != args.height.value_or(Size::Expand)) {
			height = args.height.value_or(Size::Expand);
			app->needsRelayout = true;
		}
		if (alignment != args.alignment) {
			alignment = args.alignment;
			app->needsReposition = true;
		}
		if (sizeConstraints != args.sizeConstraints.value_or(BoxConstraints{})) {
			sizeConstraints = args.sizeConstraints.value_or(BoxConstraints{});
			app->needsRelayout = true;
		}
		if (margin != args.margin.value_or(Margin{})) {
			margin = args.margin.value_or(Margin{});
			app->needsRelayout = true;
		}
		if (padding != args.padding.value_or(Padding{})) {
			padding = args.padding.value_or(Padding{});
			app->needsRelayout = true;
		}
	}

	RenderObjectWidget *RenderObject::getWidget() const {
		if (!element || !element->widget) return nullptr;
		return std::static_pointer_cast<RenderObjectWidget>(element->widget).get();
	}

	// Single Child Render Object
	vec2 SingleChildRenderObject::calculateContentSize(BoxConstraints constraints, bool final) {
		if (child) {
			return child->calculateSize(constraints, final);
		}
		return {};
	}

	void SingleChildRenderObject::positionContentAt(const Rect &newBounds) {
		if (child) {
			child->positionAt(newBounds);
		}
	}

	void SingleChildRenderObject::drawContent() {
		if (child) {
			child->draw();
		}
	}

	// Multi Child Render Object
	vec2 MultiChildRenderObject::calculateContentSize(BoxConstraints constraints, bool final) {
		vec2 contentSize{};

		if (constraints.shrinkWidth || constraints.shrinkHeight) {
			vec2 childrenMaxSize{};
			for (auto &child: children) {
				if (!child) continue;

				const auto size = child->calculateSize(constraints, final);
				childrenMaxSize.x = std::max(size.x, childrenMaxSize.x);
				childrenMaxSize.y = std::max(size.y, childrenMaxSize.y);
			}

			if (constraints.shrinkWidth)
				constraints.maxWidth = std::clamp(childrenMaxSize.x, constraints.minWidth, constraints.maxWidth);
			if (constraints.shrinkHeight)
				constraints.maxHeight = std::clamp(childrenMaxSize.y, constraints.minHeight, constraints.maxHeight);

			constraints.shrinkWidth = false;
			constraints.shrinkHeight = false;
		}

		for (auto &child: children) {
			const auto size = child->calculateSize(constraints, final);
			contentSize.x = std::max(size.x, contentSize.x);
			contentSize.y = std::max(size.y, contentSize.y);
		}

		return contentSize;
	}

	void MultiChildRenderObject::positionContentAt(const Rect &newBounds) {
		for (auto &child: children) {
			child->positionAt(newBounds);
		}
	}

	void MultiChildRenderObject::drawContent() {
		for (auto &child: children) {
			child->draw();
		}
	}

}// namespace squi::core
