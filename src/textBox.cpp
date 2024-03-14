#include "textBox.hpp"
#include "align.hpp"
#include "box.hpp"
#include "fontStore.hpp"
#include "gestureDetector.hpp"
#include "stack.hpp"
#include "textInput.hpp"


using namespace squi;

TextBox::Theme TextBox::theme{};

TextBox::operator Child() const {
	auto storage = std::make_shared<Storage>();

	return GestureDetector{
		.onUpdate = [&, storage](GestureDetector::Event event) {
			auto &box = event.widget.as<Box::Impl>();

			storage->changed = false;
			if (event.state.active) {
				if (storage->state != Storage::State::active) storage->changed = true;
				storage->state = Storage::State::active;
			} else if (event.state.hovered) {
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
		.child{
			Box{
				.widget{
					.width = 160.f,
					.height = 32.f,
				},
				.color{theme.rest},
				.borderColor{theme.border},
				.borderWidth{1.0f},
				.borderRadius{4.0f},
				.child{
					Stack{
						.children{
							Align{
								.xAlign = 0.0f,
								.child{
									TextInput{
										.widget{
											.width = Size::Expand,
											.padding = Padding{0, 11, 0, 11},
											.onInit = [](Widget &w) {
												auto font = FontStore::defaultFont;
												if (font) w.state.height = static_cast<float>(font->getLineHeight(14));
											},
											// TODO: Add support for text color change
											.onUpdate = [&, storage](Widget &w) {
												// auto &text = (Text::Impl &)w;

												if (storage->changed) {
													switch (storage->state) {
														case Storage::State::rest:
															// text.setColor(theme.text);
															// break;
														case Storage::State::hover:
															// text.setColor(theme.text);
															break;
														case Storage::State::active:
															// text.setColor(theme.text);
															w.as<TextInput::Impl>().setActive(true);
															break;
														case Storage::State::disabled:
															// text.setColor(theme.textDisabled);
															break;
													}
												}
											},
										},
										.fontSize = 14.0f,
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
												auto &box = w.as<Box::Impl>();

												if (storage->changed) {
													switch (storage->state) {
														case Storage::State::rest:
															// w.state.setHeight(1.0f);
															// box.setColor(theme.bottomBorder);
															// break;
														case Storage::State::hover:
															w.state.height = 1.0f;
															box.setColor(theme.bottomBorder);
															break;
														case Storage::State::active:
															w.state.height = 2.0f;
															box.setColor(theme.bottomBorderActive);
															break;
														case Storage::State::disabled:
															w.state.height = 0.f;
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
