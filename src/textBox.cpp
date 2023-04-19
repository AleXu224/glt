#include "textBox.hpp"
#include "align.hpp"
#include "box.hpp"
#include "fontStore.hpp"
#include "gestureDetector.hpp"
#include "stack.hpp"
#include "text.hpp"
#include "textInput.hpp"
#include <debugapi.h>


using namespace squi;

TextBox::Theme TextBox::theme{};

TextBox::operator Child() const {
	auto storage = std::make_shared<Storage>();
	constexpr std::string_view font = R"(C:\Windows\Fonts\segoeui.ttf)";

	return GestureDetector{
		.getState = [storage](auto &w, auto gd) {
			storage->gd = gd;
		},
		.child{
			Box{
				.widget{
					.width = 160.f,
					.height = 32.f,
					.margin{4},
					.padding{0, 1, 0, 1},
					.onUpdate = [&, storage](Widget &w) {
						auto &box = (Box::Impl &) w;
						auto &gd = *storage->gd;

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
						.children{
							Align{
								.xAlign = 0.0f,
								.child{
									TextInput{
										.widget{
											.width = Size::Expand,
											.padding{0, 11, 0, 11},
											.onInit = [font = font](Widget &w) {
											auto &data = w.data();
											data.sizeMode.height = static_cast<float>(FontStore::getLineHeight(font, 14)); },
											// TODO: Add support for text color change
											.onUpdate = [&, storage](Widget &w) {
											// auto &text = (Text::Impl &)w;

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
														((TextInput::Impl &)w).setActive(true);
														break;
													case Storage::State::disabled:
														// text.setColor(theme.textDisabled);
														break;
												}
											} },
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
											.height = 1.f,
											.onUpdate = [&, storage](Widget &w) {
												auto &box = (Box::Impl &) w;
												auto &widgetData = w.data();

												if (storage->changed) {
													switch (storage->state) {
														case Storage::State::rest:
															widgetData.sizeMode.height = 1.0f;
															box.setColor(theme.bottomBorder);
															break;
														case Storage::State::hover:
															widgetData.sizeMode.height = 1.0f;
															box.setColor(theme.bottomBorder);
															break;
														case Storage::State::active:
															widgetData.sizeMode.height = 2.0f;
															box.setColor(theme.bottomBorderActive);
															break;
														case Storage::State::disabled:
															widgetData.sizeMode.height = 0.0f;
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
		},
	};
}
