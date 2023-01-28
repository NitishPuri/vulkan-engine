#include "lve_renderer.h"

#include <array>
#include <cassert>
#include <stdexcept>

namespace lve {

	LveRenderer::LveRenderer(LveWindow& window, LveDevice& device) 
		: _lveWindow{ window }, _lveDevice { device } 
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	LveRenderer::~LveRenderer() {
		freeCommandBuffers();
	}

	void LveRenderer::createCommandBuffers() {
		_commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = _lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

		if (vkAllocateCommandBuffers(_lveDevice.device(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers.");
		}
	}

	void LveRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(_lveDevice.device(), _lveDevice.getCommandPool(),
			static_cast<uint32_t>(_commandBuffers.size()),
			_commandBuffers.data());
		_commandBuffers.clear();
	}

	void LveRenderer::recreateSwapChain() {
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
			std::shared_ptr<LveSwapChain> oldSwapChain = std::move(_lveSwapChain);
			_lveSwapChain = std::make_unique<LveSwapChain>(_lveDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*_lveSwapChain.get())) {
				throw std::runtime_error("Swap chain image format has chainged");
			}
		}
	}

	VkCommandBuffer LveRenderer::beginFrame() 
	{
		assert(!isFrameStarted && "Can't call begin frame while already in progress.");
		auto result = _lveSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain images.");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		return commandBuffer;
	}
	void LveRenderer::endFrame()
	{
		assert(isFrameStarted && "Can't call end frame while frame not in progress.");

		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffers.");
		}

		auto result = _lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			_lveWindow.wasWindowResized())
		{
			_lveWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain images.");
		}

		isFrameStarted = false;	
		currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void LveRenderer::beginSwapchainRenderpass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call beginSwapchainRenderpass while frame not in progress.");
		assert(commandBuffer == getCurrentCommandBuffer()
			&& "Can't begin render pass on command buffer from a different frame.");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _lveSwapChain->getRenderPass();
		renderPassInfo.framebuffer = _lveSwapChain->getFrameBuffer(currentImageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _lveSwapChain->getSwapChainExtent();


		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(_lveSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(_lveSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, _lveSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void LveRenderer::endSwapchainRenderpass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call endSwapchainRenderpass while frame not in progress.");
		assert(commandBuffer == getCurrentCommandBuffer()
			&& "Can't end render pass on command buffer from a different frame.");

		vkCmdEndRenderPass(commandBuffer);
	}
}