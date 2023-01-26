#include "first_app.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <array>

namespace lve{

	struct SimplePushConstantData {
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp() {
		loadModels();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(_lveDevice.device(), _pipelineLayout, nullptr);
	}

	void FirstApp::run() {
		while (!_lveWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(_lveDevice.device());
	}

	void FirstApp::loadModels()
	{
		std::vector<LveModel::Vertex> vertices = {
			{{0.0f, -0.5,}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5,}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5,}, {0.0f, 0.0f, 1.0f}}
		};

		_lveModel = std::make_unique<LveModel>(_lveDevice, vertices);
	}

	void FirstApp::createPipelineLayout() {

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

		if (vkCreatePipelineLayout(
			_lveDevice.device(),
			&pipelineLayoutInfo, 
			nullptr, 
			&_pipelineLayout) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}
	}

	void FirstApp::createPipeline() {
		assert(_lveSwapChain != nullptr && "Cannot create pipeline before swap chain.");
		assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout.");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = _lveSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = _pipelineLayout;
		_lvePipeline = std::make_unique<LvePipeline>(_lveDevice, 
			"shaders/vert.spv", "shaders/frag.spv", pipelineConfig);
	}

	void FirstApp::createCommandBuffers() {
		_commandBuffers.resize(_lveSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = _lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());
		
		if (vkAllocateCommandBuffers(_lveDevice.device(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers.");
		}
	}

	void FirstApp::freeCommandBuffers()
	{
		vkFreeCommandBuffers(_lveDevice.device(), _lveDevice.getCommandPool(),
			static_cast<uint32_t>(_commandBuffers.size()),
			_commandBuffers.data());
		_commandBuffers.clear();
	}

	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = _lveSwapChain->acquireNextImage(&imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain images.");
		}

		recordCommandBuffer(imageIndex);

		result = _lveSwapChain->submitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || 
			_lveWindow.wasWindowResized()) 
		{
			_lveWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain images.");
		}
	}

	void FirstApp::recreateSwapChain() {
		auto extent = _lveWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = _lveWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(_lveDevice.device());

		if (_lveSwapChain == nullptr) {
			_lveSwapChain = std::make_unique<LveSwapChain>(_lveDevice, extent);
		}
		else {
			_lveSwapChain = std::make_unique<LveSwapChain>(_lveDevice, extent, std::move(_lveSwapChain));
			if (_lveSwapChain->imageCount() != _commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		// if render pass compatible do nothing else
		createPipeline();
	}

	void FirstApp::recordCommandBuffer(int imageIndex)
	{
		static int frame = 0;
		frame = (frame + 1) % 1000;

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _lveSwapChain->getRenderPass();
		renderPassInfo.framebuffer = _lveSwapChain->getFrameBuffer(imageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _lveSwapChain->getSwapChainExtent();


		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(_lveSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(_lveSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, _lveSwapChain->getSwapChainExtent()};
		vkCmdSetViewport(_commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(_commandBuffers[imageIndex], 0, 1, &scissor);

		_lvePipeline->bind(_commandBuffers[imageIndex]);
		_lveModel->bind(_commandBuffers[imageIndex]);

		for (int j = 0; j < 4; ++j) {
			SimplePushConstantData push{};
			push.offset = { -0.5f + frame * 0.002f, -0.4f + j * 0.25f };
			push.color = { 0.0f, 0.0f, 0.2f + 0.2f * j };

			vkCmdPushConstants(_commandBuffers[imageIndex],
				_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);

			_lveModel->draw(_commandBuffers[imageIndex]);
		}

		vkCmdEndRenderPass(_commandBuffers[imageIndex]);

		if (vkEndCommandBuffer(_commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffers.");
		}

	}
}