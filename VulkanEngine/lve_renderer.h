#pragma once

#include "lve_device.h"
#include "lve_swap_chain.h"
#include "lve_window.h"

#include <memory>
#include <vector>
#include <cassert>

namespace lve {
	class LveRenderer {
	public:
		LveRenderer(LveWindow& window, LveDevice& device);
		~LveRenderer();

		LveRenderer(const LveRenderer&) = delete;
		LveRenderer& operator=(const LveRenderer&) = delete;


		VkRenderPass getSwapchainRenderpass() const { return _lveSwapChain->getRenderPass(); }
		float getAspectRatio() const { return _lveSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress.");
			return _commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress.");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwapchainRenderpass(VkCommandBuffer commandBuffer);
		void endSwapchainRenderpass(VkCommandBuffer commandBuffer);
		
	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		LveWindow& _lveWindow;
		LveDevice& _lveDevice;
		std::unique_ptr <LveSwapChain> _lveSwapChain;
		std::vector<VkCommandBuffer> _commandBuffers;

		uint32_t currentImageIndex{ 0 };
		uint32_t currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};
}
