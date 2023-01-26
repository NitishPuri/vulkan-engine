#pragma once

#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_device.h"

namespace lve {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void run();

	private:
		LveWindow _lveWindow{WIDTH, HEIGHT, "Hello Vulkan!!"};
		LveDevice _lveDevice{ _lveWindow };
		LvePipeline _lvePipeline{ 
			_lveDevice, 
			"shaders/vert.spv", 
			"shaders/frag.spv", 
			LvePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)
		};
	};
}