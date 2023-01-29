#pragma once

#include "lve_device.h"
#include "lve_camera.h"
#include "lve_game_object.h"
#include "lve_pipeline.h"
#include "lve_frame_info.h"

#include <memory>
#include <vector>

namespace lve {
	class PointLightSystem {
	public:
		PointLightSystem(LveDevice& device, VkRenderPass renderPass,
			VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		LveDevice& _lveDevice;

		std::unique_ptr<LvePipeline> _lvePipeline;
		VkPipelineLayout _pipelineLayout;
	};
}