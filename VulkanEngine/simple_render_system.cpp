#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <stdexcept>

namespace lve {

	struct SimplePushConstantData {
		glm::mat4 transform{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass) 
		: _lveDevice{device} 
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem() {
		vkDestroyPipelineLayout(_lveDevice.device(), _pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout() 
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout( _lveDevice.device(), &pipelineLayoutInfo, nullptr,
			&_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) 
	{
		assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout.");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = _pipelineLayout;
		_lvePipeline = std::make_unique<LvePipeline>(_lveDevice,
			"shaders/vert.spv", "shaders/frag.spv", pipelineConfig);
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo,std::vector<LveGameObject>& gameObjects) 
	{
		_lvePipeline->bind(frameInfo.commandBuffer);

		auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

		for (auto& object : gameObjects) {
			SimplePushConstantData push{};
			auto modelMatrix = object.transform.mat4();
			push.transform = projectionView * modelMatrix;
			push.normalMatrix = object.transform.normalMatrix();

			vkCmdPushConstants(frameInfo.commandBuffer,
				_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);

			object.model->bind(frameInfo.commandBuffer);
			object.model->draw(frameInfo.commandBuffer);
		}
	}
}