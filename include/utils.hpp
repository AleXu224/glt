#pragma once

#include "cstdint"
#include <utility>
#include <variant>
#include <vector>


namespace squi::utils {
	template<class T, class TupleType, size_t... I>
	consteval int64_t impl_getIndexFromTuple(std::index_sequence<I...> /*Indexes*/) {
		constexpr auto size = std::tuple_size_v<TupleType>;
		std::array<bool, size> solution{
			std::is_same_v<T, std::tuple_element_t<I, TupleType>>...
		};

		for (int64_t i = 0; i < size; i++) {
			if (solution.at(i)) return i;
		}
		return -1;
	}

	// Get the index of a type in a tuple
	template<class T, class TupleType>
	consteval int64_t getIndexFromTuple() {
		constexpr auto val = impl_getIndexFromTuple<T, TupleType>(std::make_index_sequence<std::tuple_size_v<TupleType>>{});
		static_assert(val != -1, "Type not found in the Tuple");
		return val;
	}

	template<template<class...> class, class>
	struct TransferParams;

	// Transfer the template parameters to another template
	template<template<class...> class To, class... Args>
	struct TransferParams<To, std::tuple<Args...>> {
		using type = To<Args...>;
	};

	template<template<class...> class, template<class...> class, class>
	struct TransferWrapParams;

	// Wrap the template parameters and transfer to another template
	template<template<class...> class To, template<class...> class Wrapper, class... Args>
	struct TransferWrapParams<To, Wrapper, std::tuple<Args...>> {
		using type = To<Wrapper<Args>...>;
	};

	template<class T>
	struct WrapperClass {
		using type = T;
	};

	template<class T, class Ret, size_t... I>
	std::vector<Ret> impl_FromIndex(std::index_sequence<I...> /*indx*/) {
		return std::vector<Ret>{(Ret{WrapperClass<std::tuple_element_t<I, T>>{}})...};
	}

	template<class T>
	TransferWrapParams<std::variant, WrapperClass, T>::type FromIndex(size_t index) {
		return impl_FromIndex<T, typename TransferWrapParams<std::variant, WrapperClass, T>::type>(std::make_index_sequence<std::tuple_size_v<T>>{}).at(index);
	}

	template<class T, class Fn, size_t... Is>
	constexpr void _iterateTuple(const T &tuple, Fn fn, std::index_sequence<Is...> /*unused*/) {
		(fn(std::get<Is>(tuple)), ...);
	}

	template<class T, class Fn>
	constexpr void iterateTuple(const T &tuple, Fn fn) {
		_iterateTuple(tuple, fn, std::make_index_sequence<std::tuple_size_v<T>>());
	}

	template<class RetType, std::ranges::range... Ranges>
	consteval auto mergeRanges(Ranges ...ranges) {
		constexpr size_t totalSize = (ranges.size() + ...);
		std::array<RetType, totalSize> ret{};
		size_t it = 0;
		(
			[&]() {
				for (const auto &elem: ranges) {
					ret[it++] = elem;
				}
			}(),
			...
		);

		return ret;
	}

	template<std::ranges::range Range>
	consteval auto evalRange(Range range) {
		constexpr size_t totalSize = range.size();
		std::array<std::decay_t<decltype(range[0])>, totalSize> ret{};
		size_t it = 0;
		for (const auto &elem: range) {
			ret[it++] = elem;
		}

		return ret;
	}

	template<std::ranges::range Range, class Fn>
	consteval auto transformRange(Range range, Fn fn) {
		constexpr size_t totalSize = range.size();
		using T = std::decay_t<decltype(fn(range[0]))>;
		std::array<T, totalSize> ret{};
		size_t it = 0;
		for (const auto &elem: range) {
			ret[it++] = fn(elem);
		}

		return ret;
	}

	template<class... Ts>
	struct overloaded : Ts... {
		using Ts::operator()...;
	};
}// namespace squi::utils