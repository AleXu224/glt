#ifndef GLT_CHILD_HPP
#define GLT_CHILD_HPP

#include <algorithm>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <xmemory>

#include "memory"
#include "type_traits"
#include "vector"


namespace squi {
	class Widget;

	using Child = std::shared_ptr<Widget>;
	using ChildRef = std::weak_ptr<Widget>;

	struct Children {
		std::vector<Child> children{};

		Children() = default;

		Children(const std::vector<Child> &children);
		Children(std::vector<Child> &&children);
		Children(const std::initializer_list<Child> &args);

		Children &operator=(const std::vector<Child> &children);

		[[nodiscard]] operator std::vector<Child> &();
		[[nodiscard]] operator std::vector<Child>() const;

		[[nodiscard]] operator bool() const;

	private:
		void filter();
	};

}// namespace squi

#endif//GLT_CHILD_HPP
