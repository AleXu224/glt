#include "widgets/navigator.hpp"

#include "widgets/stack.hpp"

namespace squi {
	void Navigator::State::initState() {
		pages.emplace_back(widget->child, std::vector<Child>{});
	}

	core::Child Navigator::State::build(const Element &) {
		if (pages.empty()) return nullptr;
		const auto &page = pages.back();
		std::vector<Child> content{};
		content.reserve(page.overlays.size() + 1);
		content.push_back(page.page);
		content.insert(content.end(), page.overlays.begin(), page.overlays.end());

		return Stack{
			.widget = widget->widget,
			.children = std::move(content),
		};
	}

	void Navigator::Context::push(const Child &child) const {
		auto nav = navigator.lock();
		if (!nav) return;
		nav->setState([&]() {
			nav->pages.emplace_back(child, std::vector<Child>{});
		});
	}

	void Navigator::Context::pushOverlay(const Child &child) const {
		auto nav = navigator.lock();
		if (!nav) return;
		if (nav->pages.empty()) return;
		nav->setState([&]() {
			nav->pages.back().overlays.emplace_back(child);
		});
	}

	void Navigator::Context::pop() const {
		auto nav = navigator.lock();
		if (!nav) return;
		if (nav->pages.empty()) return;
		auto &page = nav->pages.back();
		if (!page.overlays.empty()) {
			nav->setState([&]() {
				page.overlays.pop_back();
			});
		} else {
			nav->setState([&]() {
				nav->pages.pop_back();
			});
		}
	}

	void Navigator::Context::popOverlays() const {
		auto nav = navigator.lock();
		if (!nav) return;
		if (nav->pages.empty()) return;
		nav->setState([&]() {
			nav->pages.back().overlays.clear();
		});
	}

	void Navigator::Context::popPage() const {
		auto nav = navigator.lock();
		if (!nav) return;
		if (nav->pages.empty()) return;
		nav->setState([&]() {
			nav->pages.pop_back();
		});
	}

	Navigator::Context Navigator::of(const Element &element) {
		const Element *current = &element;
		while (current->widget->getTypeHash() != typeid(Navigator).hash_code()) {
			current = current->parent;
			if (current == current->root) throw std::runtime_error("Navigator not found");
		}
		return Context{
			.navigator = std::dynamic_pointer_cast<Navigator::State>(
				dynamic_cast<const StatefulElement *>(current)->state
			),
		};
	}
}// namespace squi