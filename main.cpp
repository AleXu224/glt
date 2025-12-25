#include "core/app.hpp"

#include "include/widgets/column.hpp"
#include "include/widgets/container.hpp"
#include "include/widgets/dropdownButton.hpp"
#include "include/widgets/expander.hpp"
#include "include/widgets/grid.hpp"
#include "include/widgets/iconButton.hpp"
#include "include/widgets/image.hpp"
#include "include/widgets/paginator.hpp"
#include "include/widgets/scrollview.hpp"
#include "include/widgets/sideNav.hpp"
#include "include/widgets/textBox.hpp"
#include "include/widgets/toggleSwitch.hpp"
#include "include/widgets/tooltip.hpp"
#include "include/widgets/topNav.hpp"
#include "widgets/animatedBox.hpp"
#include "widgets/fontIcon.hpp"
#include "widgets/liteFilter.hpp"
#include "widgets/numberBox.hpp"
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
		double number = 0.0;

		bool toggled = false;

		void initState() override {
			rotation.mount(this);
			scale.mount(this);
		}

		Child build(const Element &) override {
			return ScrollView{
				.children{
					ToggleSwitch{
						.active = toggled,
						.onToggle = [this](bool val) {
							setState([&]() {
								toggled = val;
								rotation = toggled ? 45.f : 0.f;
								scale = toggled ? 2.f : 1.f;
							});
						},
					},
					NumberBox{
						.value = number,
						.onChange = [this](double val) {
							setState([&]() {
								number = val;
							});
						},
					},
					NumberBox{
						.value = number,
						.onChange = [this](double val) {
							setState([&]() {
								number = val;
							});
						},
					},
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
					TextBox{
						.disabled = disabled,
						.controller = controller,
						.validator = [](const std::string &text) -> std::optional<std::string> {
							std::println("Validating: {}", text);
							if (text.contains("errorz")) {
								return "Error: Text contains the word 'errorz'";
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
						.child = "Test",
					},
					Button{
						.onClick = [this]() {
							setState([&]() {
								disabled = !disabled;
							});
						},
						.child = "Toggle Disabled",
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
							.child = "Toggle",
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
					Container{
						.widget{
							.width = 300.f,
							.height = 100.f,
							.padding = Padding{10.f},
						},
						.shouldClipContent = true,
						.child = Text{
							.text = "This is some text inside a container that clips its content. "
									"When the text is too long, it should be clipped to fit within the container's bounds. "
									"This ensures that any overflowing content does not disrupt the overall layout of the application.",
						},
					},
					Image{
						.image = ImageProvider{
							.key = FontStore::defaultFont.key,
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
						.child = "Toggle",
					},
					Button{
						.theme = Button::Theme::Accent(),
						.disabled = toggle,
						.child = "Button",
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
					.name = "Test Side Nav",
					.content = SideNav{
						.backAction = []() {
							std::println("Back action triggered");
						},
						.pages{
							SideNav::Page{
								.name = "Page 1",
								.content = Tooltip{
									.text = "This is a tooltip for Page 1",
									.child = Text{.text = "This is page 1"},
								},
							},
							SideNav::Page{
								.name = "Page 2",
								.content = Text{.text = "This is page 2"},
							},
							SideNav::Page{
								.name = "Page 3",
								.content = Text{.text = "This is page 3"},
							},
						},
					},
				},
				TopNav::Page{
					.name = "Test Paginator",
					.content = Paginator{
						.itemsPerPage = 10,
						.getItemCount = []() {
							return 95;
						},
						.builder = [](uint32_t offset, uint32_t count) {
							Children ret;
							for (uint32_t i = 0; i < count; i++) {
								ret.push_back(
									Text{.text = "Item " + std::to_string(offset + i + 1)}
								);
							}
							return Column{
								.spacing = 5.f,
								.children = ret,
							};
						},
					},
				},
				TopNav::Page{
					.name = "Test grid",
					.content = Grid{
						.columnCount = Grid::MinSize{110.f},
						.spacing = 10.f,
						.children{
							Box{
								.widget{
									.width = 100.f,
									.height = 100.f,
								},
								.color = Color::red,
							},
							Box{
								.widget{
									.width = 100.f,
									.height = 100.f,
								},
								.color = Color::teal,
							},
							Box{
								.widget{
									.width = 100.f,
									.height = 100.f,
								},
								.color = Color::turquoise,
							},
							Box{
								.widget{
									.width = 100.f,
									.height = 100.f,
								},
								.color = Color::yellow,
							},
						},
					},
				},
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
							.child = "Action",
						},
						.content = Column{
							.children{
								Text{.text = "This is the content of the expander."},
								Text{.text = "It can be anything you want."},
								Button{
									.child = "A button inside the expander",
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
