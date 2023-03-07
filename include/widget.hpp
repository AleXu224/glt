#ifndef SQUI_WIDGEt_HPP
#define SQUI_WIDGEt_HPP

#include "margin.hpp"
#include "vec2.hpp"
#include "sizeBehavior.hpp"
#include "vector"
#include "memory"

namespace squi {
	class Widget {
		struct Args {
			vec2 size;
			Margin margin;
			Margin padding;
            SizeBehavior sizeBehavior;
		};

        Args args;
        vec2 pos{};
        Widget *parent = nullptr;
        std::vector<std::shared_ptr<Widget>> children;
	};
}// namespace squi

#endif