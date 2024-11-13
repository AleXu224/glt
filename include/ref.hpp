#pragma once

#include "memory"

namespace squi {
	template<class T>
	struct Ref {
		Ref(T &val) : ptr(std::addressof(val)) {}

		Ref &operator=(const T &other) {
			*ptr = other;
			return *this;
		}

		T &operator*() const {
			return *ptr;
		}

		operator T &() const {
			return *ptr;
		}

		T *operator->() const {
			return ptr;
		}

        auto operator<=>(const T &other) const {
            return *ptr <=> other;
        }

	private:
		T *ptr;
	};
}// namespace squi