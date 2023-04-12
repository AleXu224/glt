#include "align.hpp"
#include "box.hpp"
#include "child.hpp"
#include "color.hpp"
#include "column.hpp"
#include "random"
#include "row.hpp"
#include "scrollable.hpp"
#include "scrollableFrame.hpp"
#include "scrollbar.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "button.hpp"
#include "performanceOverlay.hpp"
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
	// window.addChild(Box{
	// 	.widget{
	// 		.size{200, 200},
	// 		.margin{50},
	// 	},
	// 	.child = Scrollable{
	// 		.widget{
	// 			.sizeBehavior{
	// 				.horizontal = SizeBehaviorType::FillParent,
	// 				.vertical = SizeBehaviorType::FillParent,
	// 			},
	// 		},
	// 		.children{
	// 			Box{
	// 				.widget{
	// 					.size{100, 100},
	// 				},
	// 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// 			},
	// 			Box{
	// 				.widget{
	// 					.size{100, 100},
	// 				},
	// 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// 			},
	// 			Box{
	// 				.widget{
	// 					.size{100, 100},
	// 				},
	// 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// 			},
	// 			Box{
	// 				.widget{
	// 					.size{100, 100},
	// 				},
	// 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// 			},
	// 			Text{
	// 				.text = "Hello World!",
	// 				.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0),
	// 			},
	// 		},
	// 	},
	// });
	// std::string longText = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Adipiscing diam donec adipiscing tristique risus nec feugiat in fermentum. Enim lobortis scelerisque fermentum dui faucibus. Faucibus et molestie ac feugiat sed lectus vestibulum mattis ullamcorper. Quis auctor elit sed vulputate. Sed lectus vestibulum mattis ullamcorper velit sed ullamcorper. Turpis egestas sed tempus urna et pharetra. Sed velit dignissim sodales ut eu sem integer vitae justo. Nibh tellus molestie nunc non blandit massa enim. Tellus elementum sagittis vitae et leo duis ut diam. Suscipit adipiscing bibendum est ultricies. Convallis a cras semper auctor neque vitae tempus quam. A scelerisque purus semper eget duis at. Odio tempor orci dapibus ultrices in iaculis. Lorem sed risus ultricies tristique nulla aliquet enim. Luctus venenatis lectus magna fringilla urna porttitor. Accumsan lacus vel facilisis volutpat est velit egestas dui. Sem integer vitae justo eget. Quis blandit turpis cursus in hac habitasse. Duis at tellus at urna condimentum mattis pellentesque. Pharetra et ultrices neque ornare aenean euismod. Penatibus et magnis dis parturient montes. Ultrices in iaculis nunc sed augue lacus. Eget arcu dictum varius duis at consectetur lorem. Arcu non sodales neque sodales ut etiam sit amet. Nibh venenatis cras sed felis eget. Interdum consectetur libero id faucibus nisl tincidunt eget nullam. Amet nisl suscipit adipiscing bibendum est ultricies integer quis. Faucibus interdum posuere lorem ipsum dolor sit amet consectetur. Purus ut faucibus pulvinar elementum integer enim neque volutpat. Urna condimentum mattis pellentesque id nibh. Egestas diam in arcu cursus euismod. Arcu felis bibendum ut tristique et. Nulla facilisi etiam dignissim diam quis enim. Sem integer vitae justo eget magna fermentum iaculis eu non. Scelerisque varius morbi enim nunc faucibus a pellentesque sit. Fermentum odio eu feugiat pretium nibh ipsum consequat nisl vel. Nascetur ridiculus mus mauris vitae ultricies leo integer malesuada nunc. Quisque egestas diam in arcu cursus euismod. Amet est placerat in egestas erat imperdiet. Massa ultricies mi quis hendrerit dolor magna eget. Metus vulputate eu scelerisque felis. Gravida in fermentum et sollicitudin ac orci phasellus. Nunc pulvinar sapien et ligula ullamcorper malesuada proin. Adipiscing elit ut aliquam purus sit amet. Dolor purus non enim praesent elementum facilisis leo vel fringilla. Arcu felis bibendum ut tristique et egestas quis ipsum. Augue mauris augue neque gravida. Eu mi bibendum neque egestas. Nisi scelerisque eu ultrices vitae auctor eu.)";
	// window.addChild(Scrollable{
	// 	.widget{
	// 		.sizeBehavior{
	// 			.horizontal = SizeBehaviorType::FillParent,
	// 			.vertical = SizeBehaviorType::FillParent,
	// 		},
	// 	},
	// 	.children{
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 		Text{
	// 			.text{longText},
	// 			.lineWrap = true,
	// 		},
	// 	},
	// });
	window.addChild(ScrollableFrame{
		.widget{
			.sizeBehavior{.horizontal = SizeBehaviorType::FillParent, .vertical = SizeBehaviorType::FillParent},
		},
		.children{
			[&color, &rng]() {
				std::vector<Child> widgets;
				widgets.push_back(Button{
					.widget{
						.margin{16},
					},
					.text{"Some test button"},
					.style = ButtonStyle::Standard(),
				});
				for (int i = 0; i < 100; ++i) {
					widgets.push_back(Box{
						.widget{
							.size{100, 100},
						},
						.color = Color::RGBA(color(rng), color(rng), color(rng), 1.0f),
					});
				}
				return widgets;
			}(),
		},
	});
	// window.addChild(PerformanceOverlay{});
	window.run();
	return 0;
}
