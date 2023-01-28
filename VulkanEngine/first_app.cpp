#include "first_app.h"
#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace lve{

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		SimpleRenderSystem simpleRenderSystem{ _lveDevice, lveRenderer.getSwapchainRenderpass() };
		while (!_lveWindow.shouldClose()) {
			glfwPollEvents();
			if (auto commandBuffer = lveRenderer.beginFrame()) {
				lveRenderer.beginSwapchainRenderpass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				lveRenderer.endSwapchainRenderpass(commandBuffer);
				lveRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(_lveDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
		std::vector<LveModel::Vertex> vertices = {
			{{0.0f, -0.5,}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5,}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5,}, {0.0f, 0.0f, 1.0f}}
		};

		auto lveModel = std::make_shared<LveModel>(_lveDevice, vertices);

		auto triangle = LveGameObject::createGameObject();
		triangle.model = lveModel;
		triangle.color = { .1f, .8f, .1f };
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = { 2.f, .5f };
		triangle.transform2d.rotation = .25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}
}