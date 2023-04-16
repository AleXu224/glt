#include "textBox.hpp"
#include "align.hpp"
#include "box.hpp"
#include "fontStore.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "textInput.hpp"
#include <debugapi.h>


using namespace squi;

TextBox::Theme TextBox::theme{};

TextBox::operator Child() const {
	auto storage = std::make_shared<Storage>();
	constexpr std::string_view font = R"(C:\Windows\Fonts\segoeui.ttf)";

	return {
		Box{
			.widget{
				.size{160, 32},
				.margin{4},
				.padding{0, 1, 0, 1},
				.onUpdate = [&, storage](Widget &w){
					auto &box = (Box::Impl &)w;
					auto &widgetData = w.data();
					auto &gd = widgetData.gestureDetector;

					storage->changed = false;
					if (gd.active) {
						if (storage->state != Storage::State::active) storage->changed = true;
						storage->state = Storage::State::active;
					} else if (gd.hovered) {
						if (storage->state != Storage::State::hover) storage->changed = true;
						storage->state = Storage::State::hover;
					} else {
						if (storage->state != Storage::State::rest) storage->changed = true;
						storage->state = Storage::State::rest;
					}

					if (storage->changed) {
						switch (storage->state) {
							case Storage::State::rest:
								box.setColor(theme.rest);
								box.setBorderColor(theme.border);
								break;
							case Storage::State::hover:
								box.setColor(theme.hover);
								box.setBorderColor(theme.border);
								break;
							case Storage::State::active:
								box.setColor(theme.active);
								box.setBorderColor(theme.borderActive);
								break;
							case Storage::State::disabled:
								box.setColor(theme.disabled);
								box.setBorderColor(theme.border);
								break;
						}
					}
				},
			},
			.color{theme.rest},
			.borderColor{theme.border},
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
										// TODO: Add support for text color change
										.onUpdate = [&, storage](Widget &w) {
											// auto &text = (Text::Impl &)w;
											auto &data = w.data();

											if (storage->changed) {
												switch (storage->state) {
													case Storage::State::rest:
														// text.setColor(theme.text);
														break;
													case Storage::State::hover:
														// text.setColor(theme.text);
														break;
													case Storage::State::active:
														// text.setColor(theme.text);
														data.gestureDetector.active = true;
														break;
													case Storage::State::disabled:
														// text.setColor(theme.textDisabled);
														break;
												}
											}
										},
									},
									.fontSize = 14.0f,
									.font{font},
									.color{theme.text},
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
										.onUpdate = [&, storage](Widget &w){
											auto &box = (Box::Impl &)w;
											auto &widgetData = w.data();
											
											if (storage->changed) {
												switch (storage->state) {
													case Storage::State::rest:
														widgetData.size.y = 1.0f;
														box.setColor(theme.bottomBorder);
														break;
													case Storage::State::hover:
														widgetData.size.y = 1.0f;
														box.setColor(theme.bottomBorder);
														break;
													case Storage::State::active:
														widgetData.size.y = 2.0f;
														box.setColor(theme.bottomBorderActive);
														break;
													case Storage::State::disabled:
														widgetData.size.y = 0.0f;
														box.setColor(theme.bottomBorder);
														break;
												}
											}
										},
									},
									.color{theme.bottomBorder},
								},
							},
						},
					},
				},
			},
		},
	};
}
