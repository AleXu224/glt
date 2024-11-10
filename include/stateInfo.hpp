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

		void bind(Widget *w, T val) {
			w->customState.add<T>(name, std::move(val));
		}
		void bind(Widget &w, T val) {
			w.customState.add<T>(name, std::move(val));
		}
		void bind(const Child &w, T val) {
			w->customState.add<T>(name, std::move(val));
		}
	};
}// namespace squi