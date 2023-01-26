#pragma once

#include "lve_window.h"
#include "lve_pipeline.h"

namespace lve {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void run();

	private:
		LveWindow _lveWindow{WIDTH, HEIGHT, "Hello Vulkan!!"};
		LvePipeline _lvePipeline{ "shaders/vert.spv", "shaders/frag.spv" };
	};
}