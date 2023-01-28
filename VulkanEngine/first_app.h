#pragma once

#include "lve_device.h"
#include "lve_game_object.h"
#include "lve_renderer.h"
#include "lve_window.h"
#include "lve_descriptors.h"

#include <memory>
#include <vector>

namespace lve {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();

	private:
		void loadGameObjects();

		LveWindow _lveWindow{WIDTH, HEIGHT, "Hello Vulkan!!"};
		LveDevice _lveDevice{ _lveWindow };
		LveRenderer lveRenderer{ _lveWindow, _lveDevice };

		std::unique_ptr<LveDescriptorPool> globalPool{};
		LveGameObject::Map gameObjects;
	};
}