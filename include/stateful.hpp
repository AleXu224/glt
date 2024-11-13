#pragma once

#include "cstdint"
#include "functional"

namespace squi {
	class Widget;

	enum class StateImpact : uint8_t {
		NoImpact,        // By itself may not change anything but the callback could
		RedrawNeeded,    // Eg: changing the color of a box
		RepositionNeeded,// Changing the scroll on a scrollable
		RelayoutNeeded,  // Size change
	};

	namespace StatefulHelpers {
		void reDraw(Widget *);
		void reArrange(Widget *);
		void reLayout(Widget *);
	};// namespace StatefulHelpers

	template<class T, StateImpact stateImpact>
	struct Stateful {
		Stateful(const Stateful &) = default;
		Stateful(Stateful &&) = default;
		Stateful &operator=(const Stateful &other) {
			if (this != &other) *this = *other;
			return *this;
		}
		Stateful &operator=(Stateful &&) = default;
		~Stateful() = default;

		template<class... Args>
		Stateful(Widget *parent, const Args &...args) : item(args...), parent(parent) {}
		template<class... Args>
		Stateful(std::function<void(Widget &, const T &)> callback, Widget *parent, const Args &...args) : item(args...), parent(parent), callback(callback) {}

		Stateful &operator=(const T&other) {
			*this << other;
			return *this;
		}

		template<class V>
		void operator<<(const V &other) {
			if (item != other) {
				item = other;
				if constexpr (stateImpact == StateImpact::RedrawNeeded) {
					StatefulHelpers::reDraw(parent);
				} else if constexpr (stateImpact == StateImpact::RepositionNeeded) {
					StatefulHelpers::reArrange(parent);
				} else if constexpr (stateImpact == StateImpact::RelayoutNeeded) {
					StatefulHelpers::reLayout(parent);
				}
				if (callback) callback(*parent, item);
			}
		}

		const T &operator->() const
			requires(std::is_pointer_v<T>)
		{
			return item;
		}

		const T *operator->() const
			requires(!std::is_pointer_v<T>)
		{
			return std::addressof(item);
		}

		const T &operator*() const {
			return item;
		}

		operator const T &() const {
			return item;
		}

	private:
		T item;
		Widget *parent;
		std::function<void(Widget &parent, const T &)> callback{};
	};
};// namespace squi