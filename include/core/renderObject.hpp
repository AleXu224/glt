#pragma once

#include "memory"
#include "vector"


namespace squi::core {
	class RenderObject {
	public:
		RenderObject *parent = nullptr;
		std::vector<std::shared_ptr<RenderObject>> children;

		virtual ~RenderObject() = default;

		// virtual void performLayout() = 0;
		// virtual void paint() = 0;

		void appendChild(std::shared_ptr<RenderObject> child) {
			if (child->parent) {
				child->parent->removeChild(child);
			}
			children.push_back(child);
			child->parent = this;
		}

		void removeChild(std::shared_ptr<RenderObject> child) {
			auto it = std::find(children.begin(), children.end(), child);
			if (it != children.end()) {
				children.erase(it);
				child->parent = nullptr;
			}
		}
	};
}// namespace squi::core
