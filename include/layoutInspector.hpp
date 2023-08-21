#pragma once

#include "observer.hpp"
#include "widget.hpp"

namespace squi {
	struct LayoutInspector {
		// Args
		// Children &content;
		// Children &overlays;
		Observable<Child> &addedChildren;
		Observable<Child> &addedOverlays;

		struct Storage {
			// Data
			std::shared_ptr<Observable<Child>::Observer> addedChildrenObserver{};
			std::shared_ptr<Observable<Child>::Observer> addedOverlaysObserver{};

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