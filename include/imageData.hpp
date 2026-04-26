#pragma once

#include "pipeline.hpp"
#include "texturedQuad.hpp"

namespace squi {
	using ImagePipeline = glt::Engine::Pipeline<glt::Engine::TexturedQuad::Vertex, true>;
	struct ImageDataImpl {
		glt::Engine::TexturedQuad quad{glt::Engine::TexturedQuad::Args{
			.position{0, 0},
			.size{0, 0},
		}};
		std::shared_ptr<glt::Engine::SamplerUniform> sampler;
		std::shared_ptr<ImagePipeline> pipeline;
	};
}// namespace squi