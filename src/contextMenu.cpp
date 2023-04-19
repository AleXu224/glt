#include "contextMenu.hpp"
#include "box.hpp"
#include "stack.hpp"

using namespace squi;

ContextMenu::operator Child() const {
    auto storage = std::make_shared<Storage>();
    
    return Stack{
        .children{
            Box{
                .widget{
                    .width = 100.f,
                    .height = 100.f,
                },
            },
        },
    };
}
