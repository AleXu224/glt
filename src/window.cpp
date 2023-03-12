#include "window.hpp"
#include "batch.hpp"
#include "chrono"
#include "fontStore.hpp"
#include "quad.hpp"
#include "random"
#include "ranges"
#include "stdexcept"

using namespace squi;

void Window::glfwError(int id, const char *description) {
	printf("GLFW Error %d: %s\n", id, description);
}

Window::Window() : Widget(Widget::Args{}, Widget::Options{.isContainer = false, .isInteractive = false}) {
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
		auto &renderer = Renderer::getInstance();
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
		printf("Source: %d, Type: %d, Severity: %d\n", source, type, severity);
		exit(1);
	},
						   nullptr);
	glfwSwapInterval(1);
}

void Window::run() {
	Renderer &renderer = Renderer::getInstance();

	//	std::vector<Quad> quads;
	//	auto mt = std::mt19937{std::random_device{}()};
	//	auto dist = std::uniform_int_distribution<int>{0, 1920};
	//	auto dist2 = std::uniform_int_distribution<int>{0, 1080};
	//	auto colorDist = std::uniform_real_distribution<float>{0.3f, 1.0f};
	//	auto sizeDist = std::uniform_int_distribution<int>{20, 50};
	//	auto borderRadiusDist = std::uniform_int_distribution<int>{0, 25};
	//	auto burderSizeDist = std::uniform_int_distribution<int>{0, 5};
	//	constexpr size_t numQuads = 10;
	//	quads.reserve(numQuads);
	//
	//	for (size_t i = 0; i < numQuads; i++) {
	//		quads.emplace_back(Quad::Args{
	//			.pos{dist(mt), dist2(mt)},
	//			.size = glm::vec2{(float)sizeDist(mt)},
	//			.color{colorDist(mt), colorDist(mt), colorDist(mt), 1.0f},
	//			.borderColor = {1.0f, 1.0f, 1.0f, 1.0f},
	//			.borderRadius = (float)borderRadiusDist(mt),
	//			.borderSize = (float)burderSizeDist(mt),
	//		});
	//	}
	//
	//	Quad WinUI(Quad::Args{
	//		.pos{200, 200},
	//		.size{100, 100},
	//		.color{1.0f, 1.0f, 1.0f, 0.0605},
	//		.borderColor = {0.29f, 0.29f, 0.29f, 1.0f},
	//		.borderRadius = 8,
	//		.borderSize = 1,
	//	});
	//
	//	Quad WinUIBg(Quad::Args{
	//		.pos{175, 175},
	//		.size{150, 150},
	//		.color{0.1529f, 0.1607f, 0.1686f, 1.0f},
	//	});

	// TextBatch textBatch("C:\\Windows\\Fonts\\arial.ttf");
	// textBatch.createQuads("Hello World!", {0.0f, 0.0f}, {100.0f, 100.0f}, {1.0f, 1.0f, 1.0f, 1.0f});

	//	auto text3 = FontStore::generateQuads("Alt font", "C:\\Windows\\Fonts\\arialbi.ttf", 20.0f, {100.0f, 148.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
	//	auto text = FontStore::generateQuads("Font 1 %", "C:\\Windows\\Fonts\\arial.ttf", 14.0f, {100.0f, 100.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
	//	auto text2 = FontStore::generateQuads("Font 1 dar cu alta marime", "C:\\Windows\\Fonts\\arial.ttf", 20.0f, {100.0f, 124.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
	//	auto WinUIText = FontStore::generateQuads("WinUI", "C:\\Windows\\Fonts\\segoeui.ttf", 14.0f, {208.0f, 208.0f}, {1.0f, 1.0f, 1.0f, 1.0f});


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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
		glClear(GL_COLOR_BUFFER_BIT);

		//		for (auto &quad: quads) {
		//			renderer.addQuad(quad);
		//		}
		//
		//		for (auto &quad: text) {
		//			renderer.addQuad(quad);
		//		}
		//		for (auto &quad: text3) {
		//			renderer.addQuad(quad);
		//		}
		//
		//		for (auto &quad: text2) {
		//			renderer.addQuad(quad);
		//		}
		//
		//		renderer.addQuad(WinUIBg);
		//		renderer.addQuad(WinUI);
		//		for (auto &quad: WinUIText) {
		//			renderer.addQuad(quad);
		//		}

		auto &children = getChildren();

		for (auto &child: std::views::reverse(children)) {
			child->setPos({0, 0});
			child->setParent(this);
			child->update();
		}

		for (auto &child: children) {
			child->draw();
		}

		renderer.render();

		glfwSwapBuffers(window.get());
		glFlush();
	}
}