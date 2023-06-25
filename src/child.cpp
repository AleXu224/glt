#include "child.hpp"
#include "widget.hpp"
#include <algorithm>
#include <memory>
#include <utility>

using namespace squi;

Children::Children(const std::vector<Child> &children) : children(children) {
	filter();
}

Children::Children(std::vector<Child> &&children) : children(std::move(children)) {
	filter();
}

Children::Children(const std::initializer_list<Child> &args) : children(args) {
	filter();
}

Children &Children::operator=(const std::vector<Child> &children) {
	this->children = children;
	return *this;
}

Children::operator std::vector<Child> &() {
	return children;
}

Children::operator std::vector<Child>() const {
	return children;
}

Children::operator bool() const {
	return !children.empty();
}

void squi::Children::filter() {
	children.erase(
		std::remove_if(children.begin(), children.end(), [](const Child &child) {
			return !child;
		}),
		children.end());
}
