#pragma once

#include "pipeline.hpp"
#include "texturedQuad.hpp"

namespace squi {
	using ImagePipeline = Engine::Pipeline<Engine::TexturedQuad::Vertex, true>;
	struct ImageDataImpl {
		Engine::TexturedQuad quad{Engine::TexturedQuad::Args{
			.position{0, 0},
			.size{0, 0},
		}};
		std::shared_ptr<Engine::SamplerUniform> sampler;
		std::shared_ptr<ImagePipeline> pipeline;
	};
}// namespace squi