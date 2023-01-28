#pragma once

#include "lve_device.h"
#include "lve_camera.h"
#include "lve_game_object.h"
#include "lve_pipeline.h"
#include "lve_frame_info.h"

#include <memory>
#include <vector>

namespace lve {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo, std::vector<LveGameObject>& gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		LveDevice& _lveDevice;

		std::unique_ptr<LvePipeline> _lvePipeline;
		VkPipelineLayout _pipelineLayout;
	};
}