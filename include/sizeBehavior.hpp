#ifndef SQUI_SIZEBEHAVIOR_HPP
#define SQUI_SIZEBEHAVIOR_HPP

namespace squi {
	enum class SizeBehaviorType {
		None,
		FillParent,
		MatchChild,
	};

    struct SizeBehavior {
        SizeBehaviorType horizontal = SizeBehaviorType::None;
        SizeBehaviorType vertical = SizeBehaviorType::None;
    };
}

#endif