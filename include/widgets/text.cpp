#include "widgets/text.hpp"

#include "core/app.hpp"
#include "textData.hpp"
#include "utils.hpp"

#include "engine/compiledShaders/textRectfrag.hpp"
#include "engine/compiledShaders/textRectvert.hpp"

namespace squi {
	Text::TextRenderObject::TextRenderObject()
		: data(std::make_unique<TextData>()) {}

	void Text::TextRenderObject::init() {
		auto *app = this->getApp();

		this->getWidgetAs<Text>()->updateRenderObject(this);

		data->pipeline = app->pipelineStore.getPipeline(Store::PipelineProvider<TextPipeline>{
			.key = "squiTextPipeline",
			.provider = [&]() {
				return TextPipeline::Args{
					.vertexShader = Engine::Shaders::textRectvert,
					.fragmentShader = Engine::Shaders::textRectfrag,
					.instance = app->engine.instance,
				};
			},
		});

		data->sampler = app->samplerStore.getSampler(app->engine.instance, font->getTexture());
	}

	vec2 Text::TextRenderObject::calculateContentSize(BoxConstraints constraints, bool) {
		if (lineWrap || forceRegen) {
			const auto &[width, height] = font->getTextSizeSafe(
				text,
				fontSize,
				lineWrap ? std::optional<float>(constraints.shrinkWidth && lineWrap ? 0.f : constraints.maxWidth) : std::nullopt
			);
			return vec2{static_cast<float>(width), static_cast<float>(height)};
		}

		return textSize;
	}

	void Text::TextRenderObject::afterSizeCalculated() {
		if ((lineWrap && size.x != lastAvailableSpace) || forceRegen) {
			lastAvailableSpace = size.x;
			const auto lineHeight = font->getLineHeight(fontSize);

			// Will only recalculate the text layout under the following circumstances:
			// 1. The available width is smaller than the cached text width
			// 2. The cached text is wrapping (the text is occupying more than one line)
			// - This is done because it would be really difficult to figure out if a change in available width would cause a layout change in this case
			if (size.x < textSize.x || static_cast<uint32_t>(textSize.y) != lineHeight || forceRegen) {
				std::tie(data->quads, textSize.x, textSize.y) = font->generateQuads(
					text,
					fontSize,
					lastPos.rounded(),
					color,
					lineWrap ? std::optional<float>(size.x) : std::nullopt
				);
			}
		}
	}

	void Text::TextRenderObject::positionContentAt(const Rect &newBounds) {
		const auto topLeft = newBounds.posFromAlignment(alignment.value_or(Alignment::TopLeft), textSize);
		if (topLeft != lastPos) {
			const vec2 roundedPos = topLeft.rounded();
			for (auto &quadVec: data->quads) {
				for (auto &quad: quadVec) {
					quad.setPos(roundedPos);
				}
			}
			lastPos = topLeft;
		}
	}

	void Text::TextRenderObject::drawContent() {
		if (!data->pipeline) return;
		if (!data->sampler) return;

		const auto pos = getContentRect().getTopLeft();
		auto *app = this->getApp();

		data->pipeline->bindWithSampler(*data->sampler);
		const auto clipRect = app->engine.instance.scissorStack.back().logical;
		const auto minOffsetX = clipRect.left - pos.x;
		// const auto minOffsetY = clipRect.top - pos.y;
		const auto maxOffsetX = clipRect.right - pos.x;
		// const auto maxOffsetY = clipRect.bottom - pos.y;

		for (auto &quadVec: data->quads) {
			auto it = std::lower_bound(
				quadVec.begin(),
				quadVec.end(),
				minOffsetX,
				[](const auto &quad, const auto &offset) {
					return (quad.getOffset().x + quad.getSize().x) < offset;
				}
			);
			auto it2 = std::lower_bound(
				it,
				quadVec.end(),
				maxOffsetX,
				[](const auto &quad, const auto &offset) {
					return (quad.getOffset().x) < offset;
				}
			);
			for (auto &quad: std::ranges::subrange(it, it2)) {
				auto [vi, ii] = data->pipeline->getIndexes();
				data->pipeline->addData(quad.getData(vi, ii));
			}
		}
	}

	RenderObjectPtr Text::createRenderObject() {
		return std::make_shared<TextRenderObject>();
	}

	void Text::updateRenderObject(RenderObject *renderObject) const {
		if (auto *textRenderObject = dynamic_cast<TextRenderObject *>(renderObject)) {
			auto *app = renderObject->getApp();

			if (textRenderObject->text != this->text) {
				textRenderObject->text = std::string(this->text);
				textRenderObject->forceRegen = true;
				app->needsRelayout = true;
			}

			if (textRenderObject->fontSize != this->fontSize) {
				textRenderObject->fontSize = this->fontSize;
				textRenderObject->forceRegen = true;
				app->needsRelayout = true;
			}

			if (textRenderObject->lineWrap != this->lineWrap) {
				textRenderObject->lineWrap = this->lineWrap;
				textRenderObject->forceRegen = true;
				app->needsRelayout = true;
			}

			{
				auto font = std::visit(
					utils::overloaded{
						[](const FontProvider &fontProvider) {
							return FontStore::getFont(fontProvider);
						},
						[](const std::shared_ptr<FontStore::Font> &font) {
							return font;
						},
					},
					this->font
				);
				if (textRenderObject->font != font) {
					textRenderObject->font = font;
					textRenderObject->forceRegen = true;
					app->needsRelayout = true;
				}
			}

			if (textRenderObject->color != this->color) {
				textRenderObject->color = this->color;
				for (auto &quadVec: textRenderObject->data->quads) {
					for (auto &quad: quadVec) {
						quad.setColor(this->color);
					}
				}
				app->needsRedraw = true;
			}
		}
	}
}// namespace squi