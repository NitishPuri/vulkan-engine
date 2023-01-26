#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <string>

namespace lve {
	class LveWindow {
	public:
		LveWindow(int width, int height, const std::string& name);
		~LveWindow();

		LveWindow(const LveWindow&) = delete;
		LveWindow& operator=(const LveWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(_window); }

	private:
		void initWindow();


		const int _width;
		const int _height;

		std::string _windowName;
		GLFWwindow* _window;
	};
}