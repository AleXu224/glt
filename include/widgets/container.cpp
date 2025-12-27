#include "container.hpp"

#include "core/app.hpp"

namespace squi {
	void Container::ContainerRenderObject::init() {
		this->getWidgetAs<Container>()->updateRenderObject(this);
	}

	std::shared_ptr<RenderObject> Container::createRenderObject() {
		return std::make_shared<ContainerRenderObject>();
	}

	void Container::updateRenderObject(RenderObject *renderObject) const {
		if (auto *boxRenderObject = dynamic_cast<ContainerRenderObject *>(renderObject)) {
			auto *app = renderObject->getApp();

			if (boxRenderObject->shouldClipContent != shouldClipContent) {
				boxRenderObject->shouldClipContent = shouldClipContent;
				app->needsRedraw = true;
			}
		}
	}
	void Container::ContainerRenderObject::drawContent() {
		auto *app = this->getApp();
		if (shouldClipContent) {
			app->engine.instance.pushScissor(getRect().rounded());
		}
		SingleChildRenderObject::drawContent();
		if (shouldClipContent) {
			app->engine.instance.popScissor();
		}
	}
}// namespace squi