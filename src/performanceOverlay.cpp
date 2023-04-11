#include "performanceOverlay.hpp"
#include "align.hpp"
#include "box.hpp"
#include "text.hpp"
#include "column.hpp"
#include "row.hpp"
#include "renderer.hpp"

using namespace squi;

PerformanceOverlay::operator squi::Child() const {
    auto storage = std::make_shared<Storage>();

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
            .color{Color::HEX(0x00000080)},
            .borderRadius = 4,
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
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											Text::Impl &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.0f}", 1.0 / renderer.getDeltaTime().count()));
										},
									},
									.text = "60",
									.fontSize = 16,
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
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											Text::Impl &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getDeltaTime().count() * 1000));
										},
									},
									.text = "16.6ms",
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
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											Text::Impl &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getPollTime().count() * 1000));
										},
									},
									.text = "100",
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
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											Text::Impl &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getUpdateTime().count() * 1000));
										},
									},
									.text = "100",
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
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											Text::Impl &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getDrawTime().count() * 1000));
										},
									},
									.text = "10000",
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
								},
								Text{
									.widget{
										.onUpdate = [storage](Widget &widget) {
											Text::Impl &text = reinterpret_cast<Text::Impl &>(widget);
											const auto &renderer = Renderer::getInstance();
											text.setText(std::format("{:>06.2f}ms", renderer.getPresentTime().count() * 1000));
										},
									},
									.text = "10000",
								},
							},
						},
					},
				},
			},
		},
	};
}