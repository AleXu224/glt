#pragma once

#include "pipeline.hpp"
#include "textQuad.hpp"

namespace squi {
	using TextPipeline = Engine::Pipeline<Engine::TextQuad::Vertex, true>;
	struct TextData {
		std::shared_ptr<Engine::SamplerUniform> sampler{};
		std::vector<std::vector<Engine::TextQuad>> quads{};
		std::shared_ptr<TextPipeline> pipeline;
	};
}// namespace squi