#include "window.hpp"
#include "batch.hpp"
#include "textBatch.hpp"
#include "quad.hpp"
#include "stdexcept"
#include "random"

using namespace squi;

void Window::glfwError(int id, const char *description) {
	printf("GLFW Error %d: %s\n", id, description);
}

Window::Window() {
	glfwSetErrorCallback(&glfwError);
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");
		return;
	}

	window.reset(glfwCreateWindow(800, 600, "Window", nullptr, nullptr), [](GLFWwindow *window) {
		glfwDestroyWindow(window);
		glfwTerminate();
	});

	if (!window) {
		throw std::runtime_error("Failed to create window");
		return;
	}


	glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
		auto &renderer = *Renderer::get();
		renderer.updateScreenSize(width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderer.render();
		glfwSwapBuffers(window);
	});

	glfwMakeContextCurrent(window.get());
	gladLoadGL();
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
		printf("OpenGL Error: %s\n", message);
	}, nullptr);
	// glfwSwapInterval(1);
}

void Window::run() {
	QuadBatch<BoxVertex> quadBatch;
	// glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_LEQUAL);
	// glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	// glDisable(GL_CULL_FACE);
	// glEnable(GL_SAMPLE_ALPHA_TO_ONE);
	Renderer& renderer = *Renderer::get();
	Quad quad{{50.0f, 50.0f}, {500.0f, 500.0f}, {1.0f, 0.0f, 0.0f, 0.5f}, 25.0f, 10.0f, {1.0f, 1.0f, 1.0f, 1.0f}, 0.40f};
	Quad quaz{{100.0f, 100.0f}, {500.0f, 500.0f}, {0.0f, 0.5f, 0.5f, 1.0f}, 25.0f, 1.0f, {1.0f, 1.0f, 1.0f, 1.0f}, 0.5f};
	// Quad quad2{{0.0f, 0.0f}, {-5, -5}, {1.0f, 0.0f, 0.0f, 1.0f}};
	// auto mt = std::mt19937{std::random_device{}()};
	// auto dist = std::uniform_real_distribution<float>{0.0f, 800.0f};
	// auto dist2 = std::uniform_real_distribution<float>{0.0f, 600.0f};
	// auto colorDist = std::uniform_real_distribution<float>{0.3f, 1.0f};
	// std::vector<Quad> quads;
	// for (int i = 0; i < 10000; i++) {
	// 	quads.emplace_back(glm::vec2{dist(mt), dist2(mt)}, glm::vec2{200.0f, 200.0f}, glm::vec4{colorDist(mt), colorDist(mt), colorDist(mt), 1.0f});
	// }

	// TextBatch textBatch("C:\\Windows\\Fonts\\arial.ttf");
	// textBatch.createQuads("Hello World!", {0.0f, 0.0f}, {100.0f, 100.0f}, {1.0f, 1.0f, 1.0f, 1.0f});

	
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_MULTISAMPLE);
	// glEnable(GL_ALPHA_TEST);
	// glBlendEquation(GL_FUNC_ADD);
	// glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	unsigned int frames = 0;
	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window.get())) {
		double currentTime = glfwGetTime();
		frames++;
		if (currentTime - lastTime >= 1.0) {
			// printf("%f ms/frame (%d fps)\n", 1000.0 / double(frames), frames);
			glfwSetWindowTitle(window.get(), std::to_string(frames).c_str());
			frames = 0;
			lastTime += 1.0;
		}
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// glDrawArrays(GL_TRIANGLES, 0, 3);

		renderer.render();


		glfwSwapBuffers(window.get());
		// glFinish();
	}
}