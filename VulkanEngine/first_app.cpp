#include "first_app.h"
#include "simple_render_system.h"
#include "lve_camera.h"
#include "keyboard_movement_controller.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <chrono>

namespace lve{

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		SimpleRenderSystem simpleRenderSystem{ _lveDevice, lveRenderer.getSwapchainRenderpass() };

        LveCamera camera{};

        auto viewerObject = LveGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!_lveWindow.shouldClose()) {
			glfwPollEvents();
        
            auto newTime = std::chrono::high_resolution_clock::now();
            auto frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // update viewer object
            cameraController.moveInPlaneXZ(_lveWindow.getGLFWWindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, .1f, 10.f);

			if (auto commandBuffer = lveRenderer.beginFrame()) {
				lveRenderer.beginSwapchainRenderpass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				lveRenderer.endSwapchainRenderpass(commandBuffer);
				lveRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(_lveDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
		std::shared_ptr<LveModel> objModel = LveModel::createModelFromFile(_lveDevice,
			"models/smooth_vase.obj");

        auto gameObj = LveGameObject::createGameObject();
        gameObj.model = objModel;
        gameObj.transform.translation = { .0f, .0f, 2.5f };
        gameObj.transform.rotation = { .5f, .3f, .0f };
        gameObj.transform.scale = { .5f, .5f, .5f };

        gameObjects.push_back(std::move(gameObj));
	}
}