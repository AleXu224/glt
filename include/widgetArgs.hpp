#pragma once

#include "functional"
#include "margin.hpp"
#include "memory"
#include "sizeConstraints.hpp"
#include "stateContainer.hpp"
#include "variant"


namespace squi {
	class Widget;

	using Child = std::shared_ptr<Widget>;
	using ChildRef = std::weak_ptr<Widget>;
	using Children = std::vector<Child>;

	enum class Size : uint8_t {
		// Will expand to fill all the available space
		Expand,
		// Will shrink to the minimum size
		Shrink,
		// Will will let the children expand, taking up the minimum size to contain the children afterwards
		Wrap,
	};
	struct Args {
		using Width = std::variant<float, Size>;
		std::optional<Width> width{};

		using Height = std::variant<float, Size>;
		std::optional<Height> height{};

		SizeConstraints sizeConstraints{};
		std::optional<Margin> margin{};
		std::optional<Margin> padding{};

		std::vector<StateContainer> customState{};
		std::function<void(Widget &)> onInit{};
		std::function<void(Widget &)> afterInit{};
		std::function<void(Widget &)> onUpdate{};
		std::function<void(Widget &)> afterUpdate{};
		std::function<void(Widget &, vec2 &, vec2 &)> beforeLayout{};
		std::function<void(Widget &, vec2 &, vec2 &)> onLayout{};
		std::function<void(Widget &, vec2 &)> afterLayout{};
		std::function<void(Widget &, vec2 &)> onArrange{};
		std::function<void(Widget &, vec2 &)> afterArrange{};
		std::function<void(Widget &)> beforeDraw{};
		std::function<void(Widget &)> onDraw{};
		std::function<void(Widget &)> afterDraw{};
		std::function<void(Widget &, std::shared_ptr<Widget>)> onChildAdded{};
		std::function<void(Widget &, std::shared_ptr<Widget>)> onChildRemoved{};
#ifndef NDEBUG
		std::function<void()> onDebugUpdate{};
		std::function<void()> onDebugLayout{};
		std::function<void()> onDebugArrange{};
		std::function<void()> onDebugDraw{};
#endif

		[[nodiscard]] Args withDefaultWidth(const Width &newWidth) const;
		[[nodiscard]] Args withDefaultHeight(const Height &newHeight) const;
		[[nodiscard]] Args withDefaultMargin(const Margin &newMargin) const;
		[[nodiscard]] Args withDefaultPadding(const Padding &newPadding) const;
		[[nodiscard]] Args withSizeConstraints(const SizeConstraints &newSizeConstraints) const;
	};
}// namespace squi