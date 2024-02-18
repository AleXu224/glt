#pragma once
#include <array>
#include <functional>
namespace Engine {
	template<class... T>
	struct GetterSetter {
		using type = typename std::tuple_element<0, std::tuple<T...>>::type;
		GetterSetter(T &...elems) : obj(std::reference_wrapper<type>(elems)...) {}

		GetterSetter &operator=(type newValue) {
			std::apply([&](auto &...val) {
				((val.get() = newValue), ...);
			},
					   obj);
			return *this;
		}

		// template<std::enable_if<sizeof...(T) == 1>>
		operator type &() {
			// static_assert(sizeof...(T) == 1, "Can only be used if the number of elements is 1");
			return std::get<0>(obj).get();
		}

        operator const type &() const {
            return std::get<0>(obj).get();
        }

		GetterSetter(const GetterSetter &) = delete;
		GetterSetter(GetterSetter &) = delete;
		GetterSetter &operator=(const GetterSetter &) = delete;
		GetterSetter(const GetterSetter &&) = delete;
		GetterSetter(GetterSetter &&) = delete;
		GetterSetter &operator=(GetterSetter &&) = delete;
		~GetterSetter() = default;

	private:
		std::tuple<std::reference_wrapper<T>...> obj;
	};
}// namespace Engine