#pragma once

#include "pipeline.hpp"
#include "quad.hpp"

namespace squi {
	using BoxPipeline = glt::Engine::Pipeline<glt::Engine::Quad::Vertex>;

	struct BoxData {
		glt::Engine::Quad quad;
		std::shared_ptr<BoxPipeline> pipeline;
	};
}// namespace squi