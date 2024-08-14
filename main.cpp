#include "box.hpp"
#include "column.hpp"
#include "expander.hpp"
#include "gestureDetector.hpp"
#include "text.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <print>


int main(int /*unused*/, char ** /*unused*/) {
	using namespace squi;

	std::string_view str = "(ATK 1514.54 * 258.65% + Passive 2 : (Elemental Mastery 313.50 * 150.00%)) * (100.00% + Electro DMG% 46.60%) * (100.00% + Crit Rate 97.30% * Crit DMG 205.00%) * 48.70% * 90.00%";

	VoidObservable sizeChange{};

	static size_t offset = 0;

	squi::Window window{};
	window.addChild(Column{
		.children{
			// Expander{
			// 	.widget{
			// 		.margin = 8.f,
			// 	},
			// 	.icon = 0xE82F,
			// 	.heading = "This is a heading",
			// 	.caption = "This is a caption",
			// 	.child = Box{
			// 		.widget{
			// 			.width = 16.f,
			// 			.height = 16.f,
			// 		},
			// 		.color = 0xFF0000FF,
			// 	},
			// },
			GestureDetector{
				.onUpdate = [sizeChange, &str](GestureDetector::Event) {
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_DOWN)) {
						if (offset < str.size()) {
							++offset;
							sizeChange.notify();
						}
					}
					if (GestureDetector::isKeyPressedOrRepeat(GLFW_KEY_UP)) {
						if (offset > 0) {
							--offset;
							sizeChange.notify();
						}
					}
				},
				.child = Column{
					.widget{
						.width = 559.f,
						.height = Size::Shrink,
					},
					.children{
						Box{
							.widget{
								.width = Size::Wrap,
								.height = Size::Shrink,
							},
							.color{1.f, 0.f, 1.f, 1.f},
							.child = Text{
								.widget{
									.onInit = [sizeChange, &str](Widget &w) {
										w.customState.add(sizeChange.observe([&w, &str]() {
											w.as<Text::Impl>().setText(std::string_view(str.begin(), str.end() - static_cast<int64_t>(offset)));
										}));
									},
								},
								.text = str,
								.lineWrap = true,
							},
						},
					},
				},
			},
		},
	});
	window.run();
	return 0;
}
