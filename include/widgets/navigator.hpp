#pragma once

#include "core/core.hpp"

namespace squi {
	struct Navigator : StatefulWidget {
		struct Page {
			Child page;
			std::vector<Child> overlays;
		};

		// Args
		Key key;
		Args widget;
		Child child;

		struct State : WidgetState<Navigator> {
			std::vector<Page> pages;

			void initState() override;

			Child build(const Element &) override;
		};

		struct Context {
			std::weak_ptr<Navigator::State> navigator;

			void push(const Child &child) const;

			void pushOverlay(const Child &child) const;

			void pop() const;

			void popOverlays() const;
			void popOverlay() const;

			void popPage() const;
		};

		static Context of(const Element &element);
	};
}// namespace squi