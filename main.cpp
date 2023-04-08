#include "align.hpp"
#include "box.hpp"
#include "child.hpp"
#include "color.hpp"
#include "column.hpp"
#include "random"
#include "row.hpp"
#include "scrollable.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "widget.hpp"
#include "window.hpp"

int main(int, char **) {
	using namespace squi;
	squi::Window window;
	// window.addChild(Box{
	// 	.widget{
	// 		.size = {200, 200},
	// 		.margin = {10},
	// 		.padding = {10},
	// 	},
	// 	.color = Color::RGBA(0.0, 0.5, 0.5, 1.0),
	// 	.borderRadius = 10,
	// 	.child = Box{
	// 		.widget{
	// 			.size = {100, 200},
	// 			.margin = {10, 10, 10, 40},
	// 			.onInit = [](Widget &widget) {
	// 				auto &data = widget.data();
	// 				data.gestureDetector.onClick = [&data](GestureDetector &gd) {
	// 					static bool isBig = false;
	// 					if (isBig) {
	// 						data.size.x.animateTo(100, 200ms);
	// 						isBig = false;
	// 					} else {
	// 						data.size.x.animateTo(120, 200ms);
	// 						isBig = true;
	// 					}
	// 				};
	// 				data.gestureDetector.onEnter = [&widget](GestureDetector &gd) {
	// 					((Box::Impl &) widget).setColor(Color::RGBA(0.0, 0.5, 0.0, 1.0));
	// 				};
	// 				data.gestureDetector.onLeave = [&widget](GestureDetector &gd) {
	// 					((Box::Impl &) widget).setColor(Color::RGBA(0.5, 0.5, 0.5, 1.0));
	// 				};
	// 			},
	// 		},
	// 		.color = Color::RGBA(0.5, 0.5, 0.5, 1.0),
	// 		.borderWidth = 1,
	// 		.borderRadius = 100,
	// 	},
	// });

	std::mt19937 rng{1337};
	std::uniform_real_distribution<float> color{0.25, 0.75};

	// window.addChild(Column{
	// 	.widget{
	// 		.size = {200, 200},
	// 		.margin = {10},
	// 		.padding = {10},
	// 		.sizeBehavior = {
	// 			.vertical = SizeBehaviorType::FillParent,
	// 		},
	// 	},
	// 	.alignment = Column::Alignment::center,
	// 	.spacing = 10,
	// 	.children{
	// 		Box{
	// 			.widget{
	// 				.size{50, 100},
	// 			},
	// 			.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// 		},
	// 		Box{
	// 			.widget{
	// 				.size{100, 100},
	// 				.sizeBehavior = {
	// 					.vertical = SizeBehaviorType::FillParent,
	// 				},
	// 			},
	// 			.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// 		},
	// 		Box{
	// 			.widget{
	// 				.size{70, 100},
	// 			},
	// 			.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// 		},
	// 	},
	// });
	// window.addChild(Align{
	// 	.child = Box{
	// 		.widget{
	// 			.size{100, 100},
	// 		},
	// 		.color = Color::RGBA(0.5, 0.5, 0.5, 1.0),
	// 	},
	// });
	// window.addChild(Stack{
	// 	.widget{
	// 		.sizeBehavior{
	// 			.horizontal = SizeBehaviorType::FillParent,
	// 			.vertical = SizeBehaviorType::FillParent,
	// 		},
	// 	},
	// 	.children{
	// 		Box{
	// 			.widget{
	// 				.size{100, 100},
	// 			},
	// 			.color = Color::RGBA(1.0, 0.0, 0.0, 0.5),
	// 		},
	// 		Align{
	// 			.child = Box{
	// 				.widget{
	// 					.size{100, 100},
	// 				},
	// 				.color = Color::RGBA(0.0, 0.0, 1.0, 0.5),
	// 			},
	// 		},
	// 	},
	// });
	window.addChild(Box{
		.widget{
			.size{200, 200},
			.margin{50},
		},
		.child = Scrollable{
			.widget{
				.sizeBehavior{
					.horizontal = SizeBehaviorType::FillParent,
					.vertical = SizeBehaviorType::FillParent,
				},
			},
			.children{
				Box{
					.widget{
						.size{100, 100},
					},
					.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
				},
				Box{
					.widget{
						.size{100, 100},
					},
					.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
				},
				Box{
					.widget{
						.size{100, 100},
					},
					.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
				},
				Box{
					.widget{
						.size{100, 100},
					},
					.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
				},
				Text{
					.text = "Hello World!",
					.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
				},
			},
		},
	});
	// std::string longText = R"(
	// 				// window.addChild(Align{
	// // 	.child = Box{
	// // 		.widget{
	// // 			.size{100, 100},
	// // 		},
	// // 		.color = Color::RGBA(0.5, 0.5, 0.5, 1.0),
	// // 	},
	// // });
	// // window.addChild(Stack{
	// // 	.widget{
	// // 		.sizeBehavior{
	// // 			.horizontal = SizeBehaviorType::FillParent,
	// // 			.vertical = SizeBehaviorType::FillParent,
	// // 		},
	// // 	},
	// // 	.children{
	// // 		Box{
	// // 			.widget{
	// // 				.size{100, 100},
	// // 			},
	// // 			.color = Color::RGBA(1.0, 0.0, 0.0, 0.5),
	// // 		},
	// // 		Align{
	// // 			.child = Box{
	// // 				.widget{
	// // 					.size{100, 100},
	// // 				},
	// // 				.color = Color::RGBA(0.0, 0.0, 1.0, 0.5),
	// // 			},
	// // 		},
	// // 	},
	// // });
	// // window.addChild(Box{
	// // 	.widget{
	// // 		.size{200, 200},
	// // 		.margin{50},
	// // 	},
	// // 	.child = Scrollable{
	// // 		.widget{
	// // 			.sizeBehavior{
	// // 				.horizontal = SizeBehaviorType::FillParent,
	// // 				.vertical = SizeBehaviorType::FillParent,
	// // 			},
	// // 		},
	// // 		.children{
	// // 			Box{
	// // 				.widget{
	// // 					.size{100, 100},
	// // 				},
	// // 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// // 			},
	// // 			Box{
	// // 				.widget{
	// // 					.size{100, 100},
	// // 				},
	// // 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// // 			},
	// // 			Box{
	// // 				.widget{
	// // 					.size{100, 100},
	// // 				},
	// // 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// // 			},
	// // 			Box{
	// // 				.widget{
	// // 					.size{100, 100},
	// // 				},
	// // 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// // 			},
	// // 		},
	// // 	},
	// // });
	// 			)";
	// window.addChild(Column{
	// 	.widget{
	// 		.sizeBehavior{
	// 			.horizontal = SizeBehaviorType::FillParent,
	// 			.vertical = SizeBehaviorType::FillParent,
	// 		},
	// 	},
	// 	.children{
	// 		Text{
	// 			.text{longText},
	// 		},
	// 		// Text{
	// 		// 	.text{longText},
	// 		// },
	// 		// Text{
	// 		// 	.text{longText},
	// 		// },
	// 		// Text{
	// 		// 	.text{longText},
	// 		// },
	// 		// Text{
	// 		// 	.text{longText},
	// 		// },
	// 		// Text{
	// 		// 	.text{longText},
	// 		// },
	// 		// Text{
	// 		// 	.text{longText},
	// 		// },
	// 		// Text{
	// 		// 	.text{longText},
	// 		// },
	// 	},
	// });
	window.run();
	return 0;
}
