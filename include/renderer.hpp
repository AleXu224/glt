#ifndef SQUI_RENDERER_HPP
#define SQUI_RENDERER_HPP

#include "array"
#include "batch.hpp"
#include "list"
#include "shader.hpp"
#include "vertex.hpp"
#include "memory"

namespace squi {
	class Renderer {
		static std::unique_ptr<Renderer> instance;
		Batch batch{};
		Shader shader;
		glm::mat4 projectionMatrix;


	public:
		static Renderer &getInstance();

		void addQuad(Quad &quad);

		void render();

		void updateScreenSize(int width, int height);

		Renderer();
		~Renderer();
	};
}// namespace squi

#endif