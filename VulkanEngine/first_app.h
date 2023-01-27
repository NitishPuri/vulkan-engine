#pragma once

#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_swap_chain.h"
#include "lve_device.h"
#include "lve_model.h"
#include "lve_game_object.h"

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
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
		void renderGameObjects(VkCommandBuffer commandBuffer);

		LveWindow _lveWindow{WIDTH, HEIGHT, "Hello Vulkan!!"};
		LveDevice _lveDevice{ _lveWindow };
		std::unique_ptr <LveSwapChain> _lveSwapChain;
		std::unique_ptr<LvePipeline> _lvePipeline;
		VkPipelineLayout _pipelineLayout;
		std::vector<VkCommandBuffer> _commandBuffers;
		//std::unique_ptr<LveModel> _lveModel;
		std::vector<LveGameObject> gameObjects;
	};
}