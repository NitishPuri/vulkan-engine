#pragma once

#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_swap_chain.h"
#include "lve_device.h"

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
		void operator=(const FirstApp&) = delete;

		void run();

	private:
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();


		LveWindow _lveWindow{WIDTH, HEIGHT, "Hello Vulkan!!"};
		LveDevice _lveDevice{ _lveWindow };
		LveSwapChain _lveSwapChain{_lveDevice, _lveWindow.getExtent()};
		std::unique_ptr<LvePipeline> _lvePipeline;

		VkPipelineLayout _pipelineLayout;
		std::vector<VkCommandBuffer> _commandBuffers;
	};
}