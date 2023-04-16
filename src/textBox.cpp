#include "textBox.hpp"
#include "align.hpp"
#include "box.hpp"
#include "fontStore.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "textInput.hpp"
#include <debugapi.h>


using namespace squi;

TextBox::operator Child() const {
	auto storage = std::make_shared<Storage>();
	constexpr std::string_view font = R"(C:\Windows\Fonts\segoeui.ttf)";

	return {
		Box{
			.widget{
				.size{160, 32},
				.margin{4},
				.padding{0, 1, 0, 1},
			},
			.color{Color::HEX(0xFFFFFF0F)},
			.borderColor{Color::HEX(0xFFFFFF14)},
			.borderWidth = 1.0f,
			.borderRadius = 4.0f,
			.child{
				Stack{
					.widget{
						.sizeBehavior{
							.horizontal = SizeBehaviorType::FillParent,
							.vertical = SizeBehaviorType::FillParent,
						},
					},
					.children{
						Align{
							.xAlign = 0.0f,
							.child{
								TextInput{
									.widget{
										.padding{0, 11, 0, 11},
										.sizeBehavior{
											.horizontal = SizeBehaviorType::FillParent,
										},
										.onInit = [font = font](Widget &w) {
											auto &data = w.data();
											data.size.y = static_cast<float>(FontStore::getLineHeight(font, 14));
										},
									},
									.fontSize = 14.0f,
									.font{font},
									.color{Color::HEX(0xFFFFFFFF)},
								},
							},
						},
						Align{
							.xAlign = 0.0f,
							.yAlign = 1.0f,
							.child{
								Box{
									.widget{
										.size{1},
										.sizeBehavior{
											.horizontal = SizeBehaviorType::FillParent,
										},
									},
									.color{Color::HEX(0xFFFFFF8B)},
								},
							},
						},
					},
				},
			},
		},
	};
}
