#ifndef SQUI_WIDGEt_HPP
#define SQUI_WIDGEt_HPP

#include "functional"
#include "margin.hpp"
#include "memory"
#include "sizeBehavior.hpp"
#include "vec2.hpp"
#include "vector"

namespace squi {
	class Widget {
		struct Args {
			vec2 size;
			Margin margin;
			Margin padding;
			SizeBehavior sizeBehavior;
			std::function<void(Widget &)> onInit{};
			std::function<void(Widget &)> beforeUpdate{};
			std::function<void(Widget &)> onUpdate{};
			std::function<void(Widget &)> afterUpdate{};
		};

		struct Options {
			/**
			 * When true this Widget will relay all the
			 */
			bool isContainer = true;
			bool shouldUpdateChildren = true;
		};

		Args data;
		bool isContainer;
		bool shouldUpdateChildren;
		vec2 pos{};
		vec2 sizeHint{-1, -1};
		Widget *parent = nullptr;
		std::vector<std::shared_ptr<Widget>> children;

		[[nodiscard]] const Args &getData() const;
		[[nodiscard]] Args &getData();

	public:
		// Disable copy
		Widget(const Widget &) = delete;
		Widget &operator=(const Widget &) = delete;
		// Disable move
		Widget(Widget &&) = delete;
		Widget &operator=(Widget &&) = delete;

		explicit Widget(const Args &args, const Options &options);

		// Getters
		[[nodiscard]] const vec2 &getSize() const;
		[[nodiscard]] const vec2 &getPos() const;
		[[nodiscard]] const vec2 &getSizeHint() const;
		[[nodiscard]] const Margin &getMargin() const;
		[[nodiscard]] const Margin &getPadding() const;
		[[nodiscard]] const SizeBehavior &getSizeBehavior() const;
		[[nodiscard]] Widget &getParent() const;
		[[nodiscard]] const std::vector<std::shared_ptr<Widget>> &getChildren() const;

		// Setters
		void setSize(const vec2 &newSize);
		void setPos(const vec2 &newPos);
		void setSizeHint(const vec2 &newSizeHint);
		void setMargin(const Margin &newMargin);
		void setPadding(const Margin &newPadding);
		void setSizeBehavior(const SizeBehavior &newSizeBehavior);
		void setParent(Widget *newParent);
		void setChildren(const std::vector<std::shared_ptr<Widget>> &newChildren);

		// Methods
		void addChild(const std::shared_ptr<Widget>& child);
		void update();
		void draw();

		// Virtual methods
		virtual void onUpdate();
		virtual void afterChildrenUpdate();
		virtual void onDraw();
	};
}// namespace squi

#endif