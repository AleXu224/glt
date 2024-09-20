#pragma once

#include "inspectorQuad.hpp"
#include "pipeline.hpp"


namespace squi {
	using InspectorPipeline = Engine::Pipeline<Engine::InspectorQuad::Vertex>;

	struct LayoutInspectorData {
		std::shared_ptr<InspectorPipeline> pipeline;
	};
}// namespace squi