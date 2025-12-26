#include "key.hpp"

#include "element.hpp"

namespace squi::core {

	void GlobalKey::registerWithElement(const Element &element) const {
		Element::registerGlobalKey(*this, element.shared_from_this());
	}
	GlobalKey::~GlobalKey() {
		Element::unregisterGlobalKey(*this);
	}
}// namespace squi::core