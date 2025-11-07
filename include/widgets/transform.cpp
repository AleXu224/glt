#include "widgets/transform.hpp"

#include "core/app.hpp"
#include <numbers>

namespace squi {
	void Transform::TransformRenderObject::drawContent() {
		auto rect = getRect();
		auto offset = rect.getTopLeft();
		offset += origin * rect.size();

		glm::mat4 offsetMatrix{1.f};
		offsetMatrix[3][0] = -offset.x;
		offsetMatrix[3][1] = -offset.y;

		glm::mat4 scaleMatrix{1.f};
		scaleMatrix[0][0] = scale.x;
		scaleMatrix[1][1] = scale.y;

		glm::mat4 rotateMatrix{1.f};
		auto angleRad = rotate * std::numbers::pi_v<float> / 180.f;
		rotateMatrix[0][0] = std::cos(angleRad);
		rotateMatrix[1][1] = std::cos(angleRad);
		rotateMatrix[0][1] = std::sin(angleRad);
		rotateMatrix[1][0] = -std::sin(angleRad);

		glm::mat4 translateMatrix{1.f};
		translateMatrix[3][0] = translate.x;
		translateMatrix[3][1] = translate.y;

		glm::mat4 reverseOffsetMatrix{1.f};
		reverseOffsetMatrix[3][0] = offset.x;
		reverseOffsetMatrix[3][1] = offset.y;

		auto retMatrix = reverseOffsetMatrix * translateMatrix * rotateMatrix * scaleMatrix * offsetMatrix;
		auto *app = getApp();
		app->engine.instance.pushTransform(retMatrix);

		child->draw();

		app->engine.instance.popTransform();
	}
	void Transform::updateRenderObject(RenderObject *renderObject) const {
		// Update render object properties here
		if (auto *transformRenderObject = dynamic_cast<TransformRenderObject *>(renderObject)) {
			auto *app = renderObject->getApp();
			if (origin != transformRenderObject->origin) {
				transformRenderObject->origin = origin;
				app->needsRedraw = true;
			}
			if (translate != transformRenderObject->translate) {
				transformRenderObject->translate = translate;
				app->needsRedraw = true;
			}
			if (scale != transformRenderObject->scale) {
				transformRenderObject->scale = scale;
				app->needsRedraw = true;
			}
			if (rotate != transformRenderObject->rotate) {
				transformRenderObject->rotate = rotate;
				app->needsRedraw = true;
			}
		}
	}
}// namespace squi