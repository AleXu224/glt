#include "performanceOverlay.hpp"
#include "align.hpp"
#include "box.hpp"
#include "column.hpp"
#include "renderer.hpp"
#include "row.hpp"
#include "text.hpp"
#include <string_view>


using namespace squi;

PerformanceOverlay::operator squi::Child() const {
	auto storage = std::make_shared<Storage>();
	constexpr std::string_view font = R"(C:\Windows\Fonts\segoeui.ttf)";

	return Align{
		.xAlign = 1,
		.yAlign = 1,
		.child = Box{
			.widget{
				.margin{16},
				.padding{8},
				.sizeBehavior{
					.horizontal = SizeBehaviorType::MatchChild,
					.vertical = SizeBehaviorType::MatchChild,
				},
			},
			.color{Color::HEX(0xFFFFFF0D)},
			.borderColor{Color::HEX(0x0000001A)},
			.borderWidth = 1.0f,
			.borderRadius = 4,
			.borderPosition = Box::BorderPosition::outset,
			.child{
				Column{
					.widget{
						.sizeBehavior{
							.horizontal = SizeBehaviorType::MatchChild,
							.vertical = SizeBehaviorType::MatchChild,
						},
					},
					.alignment = Column::Alignment::right,
					.children = {
						Row{
							.widget{
								.sizeBehavior{
									.horizontal = SizeBehaviorType::MatchChild,
									.vertical = SizeBehaviorType::MatchChild,
								},
							},
							.children = {
								Text{
									.text = "FPS: ",
									.fontSize = 16,
									.fontPath{font},
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											auto &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>08.0f}", 1.0 / renderer.getDeltaTime().count()));
										},
									},
									.text = "60",
									.fontSize = 16,
									.fontPath{font},
								},
							},
						},
						Row{
							.widget{
								.sizeBehavior{
									.horizontal = SizeBehaviorType::MatchChild,
									.vertical = SizeBehaviorType::MatchChild,
								},
							},
							.children = {
								Text{
									.text = "Frame time: ",
									.fontPath{font},
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											auto &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getDeltaTime().count() * 1000));
										},
									},
									.text = "16.6ms",
									.fontPath{font},
								},
							},
						},
						Row{
							.widget{
								.sizeBehavior{
									.horizontal = SizeBehaviorType::MatchChild,
									.vertical = SizeBehaviorType::MatchChild,
								},
							},
							.children = {
								Text{
									.text = "Poll time: ",
									.fontPath{font},
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											auto &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getPollTime().count() * 1000));
										},
									},
									.text = "100",
									.fontPath{font},
								},
							},
						},
						Row{
							.widget{
								.sizeBehavior{
									.horizontal = SizeBehaviorType::MatchChild,
									.vertical = SizeBehaviorType::MatchChild,
								},
							},
							.children = {
								Text{
									.text = "Update time: ",
									.fontPath{font},
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											auto &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getUpdateTime().count() * 1000));
										},
									},
									.text = "100",
									.fontPath{font},
								},
							},
						},
						Row{
							.widget{
								.sizeBehavior{
									.horizontal = SizeBehaviorType::MatchChild,
									.vertical = SizeBehaviorType::MatchChild,
								},
							},
							.children = {
								Text{
									.text = "Draw time: ",
									.fontPath{font},
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											auto &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getDrawTime().count() * 1000));
										},
									},
									.text = "10000",
									.fontPath{font},
								},
							},
						},
						Row{
							.widget{
								.sizeBehavior{
									.horizontal = SizeBehaviorType::MatchChild,
									.vertical = SizeBehaviorType::MatchChild,
								},
							},
							.children = {
								Text{
									.text = "Present time: ",
									.fontPath{font},
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											auto &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getPresentTime().count() * 1000));
										},
									},
									.text = "10000",
									.fontPath{font},
								},
							},
						},
					},
				},
			},
		},
	};
}