#pragma once

#include "engine/inspectorQuad.hpp"
#include "engine/pipeline.hpp"
#include "observer.hpp"
#include "widget.hpp"

namespace squi {
	struct LayoutInspector {
		// Args
		Observable<Child> addedChildren;
		Observable<Child> addedOverlays;

		using InspectorPipeline = Engine::Pipeline<Engine::InspectorQuad::Vertex>;
		static InspectorPipeline *pipeline;

		struct Storage {
			// Data
			Observer<Child> addedChildrenObserver{};
			Observer<Child> addedOverlaysObserver{};

			ChildRef activeButton{};
			ChildRef selectedWidget{};
			ChildRef hoveredWidget{};

			ChildRef contentStack{};
			ChildRef overlayStack{};
			bool shouldUpdate = true;
			bool selectedWidgetChanged = false;
			bool pauseUpdates = false;
			bool pauseUpdatesChanged = false;
			bool pauseLayout = false;
			bool pauseLayoutChanged = false;
		};

		operator Child() const;
	};
}// namespace squi