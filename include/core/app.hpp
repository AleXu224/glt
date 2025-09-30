#pragma once

#include "core/animationController.hpp"
#include "core/inputState.hpp"
#include "engine/engine.hpp"
#include "inputQueue.hpp"
#include "store/pipeline.hpp"
#include "store/sampler.hpp"
#include "widget.hpp"
#include <set>
#include <unordered_set>


namespace squi::core {
	struct App;
	struct RootRenderObject : SingleChildRenderObject {
		App *app = nullptr;
	};

	struct RootWidget : RenderObjectWidget {
		Key key;
		App *app = nullptr;
		std::shared_ptr<RootRenderObject> rootRenderObject;
		WidgetPtr child;

		std::shared_ptr<RenderObject> createRenderObject() const {
			return rootRenderObject;
		}

		void updateRenderObject(RenderObject *renderObject) const {
			// Update render object properties here
		}

		struct Element : SingleChildRenderObjectElement {
			App *app = nullptr;

			Element(const std::shared_ptr<RootWidget> &widget) : SingleChildRenderObjectElement(widget), app(widget->app) {}

			Child build() override {
				if (auto rootWidget = std::static_pointer_cast<RootWidget>(widget)) {
					return rootWidget->child;
				}
				return nullptr;
			}

			void mount(::squi::core::Element *parent, size_t index, size_t depth) override;
			void unmount() override;
		};
	};

	struct App {
		Engine::WindowOptions windowOptions{
			.name = "Squi App",
		};
		Engine::Runner engine{
			windowOptions
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

		std::chrono::steady_clock::time_point frameStartTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> deltaTime = 0ms;

		static inline std::mutex windowMapMtx{};
		static inline std::mutex pollMtx{};
		static inline std::unordered_map<GLFWwindow *, App *> windowMap{};
		static inline std::vector<App *> windowsToDestroy{};

		static constexpr auto elemComp = [](Element *e1, Element *e2) {
			return e1->depth < e2->depth;
		};
		std::set<Element *, decltype(elemComp)> dirtyElements{elemComp};
		std::unordered_set<AnimationController *> runningAnimations{};
		std::vector<std::function<void()>> postLayoutTasks{};

		std::shared_ptr<RootRenderObject> rootRenderObject = [this]() {
			auto ret = std::make_shared<RootRenderObject>();
			ret->app = this;
			return ret;
		}();

		ElementPtr rootElement = Child(RootWidget{.app = this, .rootRenderObject = rootRenderObject, .child = child})->_createElement();

		void run();
	};
}// namespace squi::core