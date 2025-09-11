#pragma once

#include "child.hpp"
#include "key.hpp"
#include "renderObject.hpp"
#include <concepts>
#include <vector>


namespace squi::core {
	template<class T>
	concept HasKey = requires(T a) {
		requires std::is_same_v<Key, std::remove_cvref_t<decltype(a.key)>>;
	};

	template<class T>
	concept HasElement = requires(T a) {
		typename std::remove_cvref_t<T>::Element;
		requires std::is_base_of_v<Element, typename std::remove_cvref_t<T>::Element>;
	};

	template<class T>
	concept HasWidgetArgs = requires(T a) {
		{ a.widget } -> std::same_as<Args &>;
	};

	template<class T>
	concept HasWidgetArgsGetter = requires(T a) {
		{ a.getArgs() } -> std::same_as<Args>;
	};

	template<class T>
	concept HasChild = requires(T a) {
		{ a.child } -> std::same_as<Child>;
	};

	template<class T>
	concept HasChildren = requires(T a) {
		{ a.children } -> std::same_as<std::vector<Child>>;
	};

	template<class T>
	concept StateLike = requires(T a) {
		{ a.build(std::declval<const Element &>()) } -> std::same_as<WidgetPtr>;
	};

	template<class T>
	concept StatefulWidgetLike = requires(T a) {
		typename std::remove_cvref_t<T>::State;
		requires StateLike<typename std::remove_cvref_t<T>::State>;
	};

	template<class T>
	concept StatelessWidgetLike = requires(T a) {
		{ a.build(std::declval<const Element &>()) } -> std::same_as<WidgetPtr>;
	};

	template<class T>
	concept RenderObjectWidgetLike = requires(T a) {
		{ a.createRenderObject() } -> std::same_as<std::shared_ptr<RenderObject>>;
		{ a.updateRenderObject(std::declval<RenderObject *>()) } -> std::same_as<void>;
	};
}// namespace squi::core