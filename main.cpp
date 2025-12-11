#include "core/app.hpp"

#include "include/widgets/column.hpp"
#include "include/widgets/dropdownButton.hpp"
#include "include/widgets/expander.hpp"
#include "include/widgets/iconButton.hpp"
#include "include/widgets/scrollview.hpp"
#include "include/widgets/textBox.hpp"
#include "include/widgets/topNav.hpp"
#include "widgets/animatedBox.hpp"
#include "widgets/fontIcon.hpp"
#include "widgets/liteFilter.hpp"
#include "widgets/toggleButton.hpp"
#include "widgets/transform.hpp"


using namespace squi::core;
using namespace squi;

struct ColorChanger : StatefulWidget {
	// Args
	Key key;
	float size = 50.f;

	struct State : WidgetState<ColorChanger> {
		Color color = Color::royalblue;
		float width = 50.f;
		bool expanded = false;

		Child build(const Element &) override {
			return Gesture{
				.onClick = [this](const auto &) {
					setState([this]() {
						expanded = !expanded;
						color = expanded ? Color::black : Color::royalblue;
						width = expanded ? 100.f : 50.f;
					});
				},
				.child = AnimatedBox{
					.widget{
						.width = width,
						.height = widget->size,
					},
					.duration = 500ms,
					.color = color,
				},
			};
		}
	};
};

struct Test : StatefulWidget {
	// Args
	Key key;

	struct State : WidgetState<Test> {
		TextInput::Controller controller{"Hello"};
		Animated<float> rotation{.from = 0.f};
		Animated<float> scale{.from = 1.f};
		bool disabled = false;

		bool toggled = false;

		void initState() override {
			rotation.mount(this);
			scale.mount(this);
		}

		Child build(const Element &) override {
			return Column{
				.children{
					TextBox{
						.disabled = disabled,
						.controller = controller,
						.validator = [](const std::string &text) -> std::optional<std::string> {
							std::println("Validating: {}", text);
							if (text.contains("error")) {
								return "Error: Text contains the word 'error'";
							}
							return {};
						},
					},
					Button{
						.onClick = [this]() {
							setState([&]() {
								// remove the last character if it exists
								if (!controller.getText().empty()) {
									controller.setText(controller.getText().substr(0, controller.getText().size() - 1));
								}
							});
						},
						.content = "Test",
					},
					Button{
						.onClick = [this]() {
							setState([&]() {
								disabled = !disabled;
							});
						},
						.content = "Toggle Disabled",
					},
					Transform{
						.scale = vec2{scale},
						.rotate = rotation,
						.child = ToggleButton{
							.active = toggled,
							.onToggle = [this](bool active) {
								setState([&]() {
									toggled = active;
									rotation = toggled ? 45.f : 0.f;
									scale = toggled ? 2.f : 1.f;
								});
							},
							.content = "Toggle",
						},
					},
					Box{
						.widget{
							.width = Size::Wrap,
							.height = Size::Wrap,
						},
						.color = Color::red,
						.child = FontIcon{
							.icon = 0xe5d2,
						},
					},
					DropdownButton{
						.items{
							ContextMenu::Button{
								.text = "some button",
							},
							ContextMenu::Button{
								.text = "some other button",
							},
							ContextMenu::Divider{},
							ContextMenu::Toggle{
								.text = "some toggle button",
								.value = true,
								.callback = [](bool val) {
									std::println("Toggle is now: {}", val);
								},
							},
							ContextMenu::Toggle{
								.text = "some other toggle button",
								.value = false,
								.callback = [](bool val) {
									std::println("Other toggle is now: {}", val);
								},
							},
						},
					},
				},
			};
		}
	};
};

struct Test2 : StatefulWidget {
	// Args
	Key key;

	struct State : WidgetState<Test2> {
		bool toggle = false;
		Button::ButtonStatus status = Button::ButtonStatus::resting;

		Child build(const Element &) override {
			return ScrollView{
				.direction = Axis::Horizontal,
				.spacing = 2.f,
				.children{
					Button{
						.onClick = [this]() {
							setState([this]() {
								toggle = !toggle;
							});
						},
						.content = "Toggle",
					},
					Button{
						.theme = Button::Theme::Accent(),
						.disabled = toggle,
						.content = "Button",
					},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
					ColorChanger{},
				},
			};
		}
	};
};

int main(int /*unused*/, char ** /*unused*/) {

	App app{
		.windowOptions{
			.width = 1280,
			.height = 720,
		},
		.child = TopNav{
			.pages{
				TopNav::Page{
					.name = "Test lite filter",
					.content = LiteFilter{
						.items{
							LiteFilter::Item{
								.name = "Item 1",
								.onUpdate = [](bool selected) {
									std::println("Item 1 selected: {}", selected);
								},
							},
							LiteFilter::Item{
								.name = "Item 2",
								.onUpdate = [](bool selected) {
									std::println("Item 2 selected: {}", selected);
								},
							},
							LiteFilter::Item{
								.name = "Item 3",
								.onUpdate = [](bool selected) {
									std::println("Item 3 selected: {}", selected);
								},
							},
						},
					},
				},
				TopNav::Page{
					.name = "Test expander",
					.content = Expander{
						.icon = FontIcon{
							.icon = 0xe5d2,
						},
						.title = "Expander Title",
						.subtitle = "This is a subtitle",
						.action = Button{
							.theme = Button::Theme::Accent(),
							.content = "Action",
						},
						.content = Column{
							.children{
								Text{.text = "This is the content of the expander."},
								Text{.text = "It can be anything you want."},
								Button{
									.content = "A button inside the expander",
								},
								IconButton{},
							},
						},
					},
				},
				TopNav::Page{
					.name = "Test 1",
					.content = Test{},
				},
				TopNav::Page{
					.name = "Test 2",
					.content = Test2{},
				},
				TopNav::Page{
					.name = "Test 3",
					.content = Test{},
				},
			},
		},
	};
	app.run();

	return 0;
}
