#include "first_app.h"


namespace lve{
	void FirstApp::run() {
		while (!_lveWindow.shouldClose()) {
			glfwPollEvents();
		}
	}
}