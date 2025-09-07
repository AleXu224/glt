#pragma once

#include "engine/engine.hpp"
#include "inputQueue.hpp"
#include "inputState.hpp"
#include "store/pipeline.hpp"
#include "store/sampler.hpp"
#include "widget.hpp"


namespace squi::core {
	struct RootRenderObject : SingleChildRenderObject {
	};

	struct RootWidget : RenderObjectWidget {
		std::shared_ptr<RootRenderObject> rootRenderObject;
		WidgetPtr child;

		std::shared_ptr<RenderObject> createRenderObject() const {
			return rootRenderObject;
		}

		void updateRenderObject(RenderObject *renderObject) const {
			// Update render object properties here
		}

		struct Element : RenderObjectElement {
			Element(const RenderObjectWidgetPtr &widget) : RenderObjectElement(widget) {}

			void mount(::squi::core::Element *parent) override;
			void unmount() override;
		};
	};

	struct App {
		Engine::Runner engine{
			Engine::WindowOptions{
				.name = "Squi App",
			},
		};

		Child child;

		Store::Sampler samplerStore{};
		Store::Pipeline pipelineStore{};
		InputState inputState{};

		InputQueue inputQueue{};

		bool needsRedraw = true;
		bool needsRelayout = true;
		bool needsReposition = true;
		bool drewLastFrame = false;
		std::function<void(bool)> maximizeCallback{};
		std::function<void(uint32_t, uint32_t)> resizeCallback{};

		static inline std::mutex windowMapMtx{};
		static inline std::mutex pollMtx{};
		static inline std::unordered_map<GLFWwindow *, App *> windowMap{};
		static inline std::vector<App *> windowsToDestroy{};

		std::shared_ptr<RootRenderObject> rootRenderObject = std::make_shared<RootRenderObject>();

		ElementPtr rootElement = Child(RootWidget{.rootRenderObject = rootRenderObject, .child = child})->_createElement();

		void run();
	};
}// namespace squi::core