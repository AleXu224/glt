#pragma once

#include "widget.hpp"

namespace squi {

	template<class T>
	struct StateInfo {
		using Type = T;

		std::string name = typeid(T).name();

		T &of(Widget *w) const {
			return w->customState.get<T>(name);
		}
		T &of(Widget &w) const {
			return w.customState.get<T>(name);
		}
		T &of(const Child &w) const {
			return w->customState.get<T>(name);
		}

		void bind(Widget *w, T val) const {
			w->customState.add<T>(name, val);
		}
		void bind(Widget &w, T val) const {
			w.customState.add<T>(name, val);
		}
		void bind(Child &w, T val) const {
			w->customState.add<T>(name, val);
		}
		template<class V>
		void bind(V &&, T val) const = delete;
	};
}// namespace squi