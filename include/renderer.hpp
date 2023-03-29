#ifndef SQUI_RENDERER_HPP
#define SQUI_RENDERER_HPP

#include "array"
#include "batch.hpp"
#include "list"
#include "shader.hpp"
#include "vertex.hpp"
#include "memory"
#include "chrono"

namespace squi {
	class Renderer {
		static std::unique_ptr<Renderer> instance;
		Batch batch{};
		Shader shader;
		glm::mat4 projectionMatrix{};
		std::chrono::duration<double> deltaTime = std::chrono::duration<double>::zero();
		std::chrono::time_point<std::chrono::steady_clock> currentFrameTime = std::chrono::steady_clock::now();

	public:
		static Renderer &getInstance();

		void addQuad(Quad &quad);

		void render();

		void updateScreenSize(int width, int height);
		void updateDeltaTime(std::chrono::duration<double> time);
		void updateCurrentFrameTime(std::chrono::time_point<std::chrono::steady_clock> time);

		[[nodiscard]] std::chrono::duration<double> getDeltaTime() const;
		[[nodiscard]] std::chrono::time_point<std::chrono::steady_clock> getCurrentFrameTime() const;

		Renderer();
		~Renderer();
	};
}// namespace squi

#endif