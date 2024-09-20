#pragma once

#include "pipeline.hpp"
#include "quad.hpp"

namespace squi {
	using BoxPipeline = Engine::Pipeline<Engine::Quad::Vertex>;

	struct BoxData {
		Engine::Quad quad;
		std::shared_ptr<BoxPipeline> pipeline;
	};
}// namespace squi