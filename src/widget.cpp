#include "widget.hpp"

using namespace squi;

Widget::Widget(const Args &args, bool isContainer) : args(args), isContainer(isContainer) {
    if (args.onInit) {
        args.onInit(this);
    }
}