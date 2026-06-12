#include "./textEditor.hpp"

namespace squi {
	struct MultilineTextEditor : public TextEditor {
		void handleEnter(const Gesture::State &state) override;

		void handleUpArrow(const Gesture::State &state) override;
		void handleDownArrow(const Gesture::State &state) override;
		void handleLeftArrow(const Gesture::State &state) override;
		void handleRightArrow(const Gesture::State &state) override;

		void handleHome(const Gesture::State &state) override;
		void handleEnd(const Gesture::State &state) override;
	};
}// namespace squi