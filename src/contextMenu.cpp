#include "contextMenu.hpp"

using namespace squi;

ContextMenu::operator Child() const {
    auto storage = std::make_shared<Storage>();
    
    return Child(std::make_shared<Widget>(this->widget));
}
