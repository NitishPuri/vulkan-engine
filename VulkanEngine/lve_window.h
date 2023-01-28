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
		LveWindow operator=(const LveWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(_window); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(_width), static_cast<uint32_t>(_height) }; }
		bool wasWindowResized() { return _framebufferResized; }
		void resetWindowResizedFlag() { _framebufferResized = false; }
		GLFWwindow* getGLFWWindow() const { return _window; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void framebufferResizedCallback(GLFWwindow* window, int width, int height);
		void initWindow();


		int _width;
		int _height;
		bool _framebufferResized = false;

		std::string _windowName;
		GLFWwindow* _window;
	};
}