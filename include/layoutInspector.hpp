#pragma once

#include "observer.hpp"
#include "widgetArgs.hpp"


namespace squi {
	struct LayoutInspectorData;
	
	struct LayoutInspector {
		// Args
		Observable<Child> addedChildren;
		Observable<Child> addedOverlays;

		struct Storage {
			// Data
			std::unique_ptr<LayoutInspectorData> data{};

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