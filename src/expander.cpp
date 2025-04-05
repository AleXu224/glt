#include "expander.hpp"
#include "align.hpp"
#include "box.hpp"
#include "button.hpp"
#include "column.hpp"
#include "fontIcon.hpp"
#include "row.hpp"
#include "text.hpp"
#include "utils.hpp"
#include <utf8/cpp20.h>


using namespace squi;

namespace {
	struct ExpanderButton {
		// Args
		Observable<bool> expandedEvent;
		Observable<ButtonState> stateEvent;
		std::shared_ptr<Expander::Storage> storage;

		struct Storage {
			// Data
		};

		operator squi::Child() const {
			auto style = ButtonStyle::Subtle();

			return Button{
				.widget{
					.width = 32.f,
					.height = 32.f,
				},
				.style = style,
				.onClick = [expandedEvent = expandedEvent, storage = storage](auto) {
					expandedEvent.notify(storage->expanded = !storage->expanded);
				},
				.child = Align{
					.child = FontIcon{
						.textWidget{
							.onInit = [expandedEvent = expandedEvent](Widget &w) {
								observe(w, expandedEvent, [&w](bool newVal) {
									auto &text = w.as<Text::Impl>();

									if (newVal)
										text.setText(utf8::utf32to8(std::u32string{0xe316}));
									else
										text.setText(utf8::utf32to8(std::u32string{0xe313}));
								});
							},
						},
						.icon = 0xe313,
					},
				},
			};
		}
	};
}// namespace

Expander::operator Child() const {
	auto storage = std::make_shared<Storage>();

	Observable<bool> expandedEvent;

	return Column{
		.widget = widget.withDefaultHeight(Size::Shrink),
		.children{
			Box{
				.widget{
					.height = 64.f,
					.onInit = [expandedEvent](Widget &w) {
						observe(w, expandedEvent, [&](bool newVal) {
							auto &box = w.as<Box::Impl>();
							if (newVal)
								box.setBorderRadius(BorderRadius{4.f}.withBottom(0.f));
							else
								box.setBorderRadius(4.f);
						});
					},
				},
				.color = 0xFFFFFF0D,
				.borderColor = 0x0000001A,
				.borderWidth{1.f},
				.borderRadius{4.f},
				.borderPosition = Box::BorderPosition::outset,
				.child = Row{
					.widget{
						.padding = Padding{16.f, 14.f},
					},
					.alignment = Row::Alignment::center,
					.spacing = 16.f,
					.children{
						std::visit(//
							utils::overloaded{
								[](char32_t icon) -> Child {
									return FontIcon{
										.icon = icon,
										.size = 16.f,
									};
								},
								[](Child child) {
									return child;
								},
							},
							icon
						),
						Column{
							.widget{
								.height = Size::Shrink,
							},
							.children{
								std::visit(//
									utils::overloaded{
										[](std::string_view str) -> Child {
											return Text{
												.text = str,
												.fontSize = 14.f,
											};
										},
										[](const Child &child) {
											return child;
										},
									},
									heading
								),
								std::visit(//
									utils::overloaded{
										[](std::string_view str) -> Child {
											if (str.empty()) return Child{};
											return Text{
												.text = str,
												.fontSize = 12.f,
												.lineWrap = true,
												.color = 0xFFFFFFC8,
											};
										},
										[](const Child &child) -> Child {
											return child;
										},
									},
									caption
								),
							},
						},
						actions.empty()//
							? Child{}
							: Row{
								  .widget{
									  .width = Size::Shrink,
									  .height = Size::Shrink,
								  },
								  .spacing = 4.f,
								  .children = actions,
							  },
						expandedContent && !alwaysExpanded//
							? ExpanderButton{
								  .expandedEvent = expandedEvent,
								  .storage = storage,
							  }
							: Child{},
					},
				},
			},
			Box{
				.widget{
					.onInit = [expandedEvent, alwaysExpanded = alwaysExpanded, expandedContentExists = static_cast<bool>(expandedContent)](Widget &w) {
						observe(w, expandedEvent, [&w](bool newVal) {
							w.flags.visible = newVal;
						});
						w.flags.visible = alwaysExpanded && expandedContentExists;
					},
				},
				.color = Color::css(0xffffff, 0.0326f),
				.borderColor = Color::css(0x0, 0.1f),
				.borderWidth = BorderWidth{1.f}.withTop(0.f),
				.borderRadius = BorderRadius::Bottom(4.f),
				.borderPosition = Box::BorderPosition::outset,
				.child = expandedContent,
			},
		},
	};
}
