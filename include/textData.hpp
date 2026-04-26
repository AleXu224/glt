#pragma once

#include "pipeline.hpp"
#include "textQuad.hpp"

namespace squi {
	using TextPipeline = glt::Engine::Pipeline<glt::Engine::TextQuad::Vertex, true>;
	struct TextData {
		std::shared_ptr<glt::Engine::SamplerUniform> sampler{};
		std::vector<std::vector<glt::Engine::TextQuad>> quads{};
		std::shared_ptr<TextPipeline> pipeline;
	};
}// namespace squi