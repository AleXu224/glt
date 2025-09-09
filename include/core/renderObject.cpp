#include "renderObject.hpp"

#include "algorithm"
#include "core/app.hpp"
#include "utils.hpp"


namespace squi::core {
	// Render Object
	App *RenderObject::getApp() {
		auto obj = this;
		while (obj->parent != nullptr && obj->parent != obj) {
			obj = obj->parent;
		}
		auto root = dynamic_cast<RootRenderObject *>(obj);
		assert(root != nullptr);
		return root->app;
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

		const auto contentSize = calculateContentSize(extConstraints.withoutPadding(padding), final);
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
		}

		return size + marginOffset;
	}

	vec2 RenderObject::calculateContentSize(BoxConstraints constraints, bool final) {
		return {};
	}

	void RenderObject::positionAt(const vec2 &newPos) {
		pos = newPos + margin.getPositionOffset();
		positionContentAt(pos + padding.getPositionOffset());
	}

	void RenderObject::draw() {
		drawSelf();
		drawContent();
	}

	// Single Child Render Object
	vec2 SingleChildRenderObject::calculateContentSize(BoxConstraints constraints, bool final) {
		if (child) {
			return child->calculateSize(constraints, final);
		}
		return {};
	}

	void SingleChildRenderObject::positionContentAt(const vec2 &newPos) {
		if (child) {
			child->positionAt(newPos);
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

		for (auto &child: children) {
			const auto size = child->calculateSize(constraints, final);
			contentSize.x = std::max(size.x, contentSize.x);
			contentSize.y = std::max(size.y, contentSize.y);
		}

		return contentSize;
	}

	void MultiChildRenderObject::positionContentAt(const vec2 &newPos) {
		for (auto &child: children) {
			child->positionAt(newPos);
		}
	}

	void MultiChildRenderObject::drawContent() {
		for (auto &child: children) {
			child->draw();
		}
	}

}// namespace squi::core