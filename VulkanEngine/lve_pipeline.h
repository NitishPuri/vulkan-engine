#pragma once

#include "lve_device.h"

#include <string>
#include <vector>

namespace lve {
	struct PipelineConfigInfo {
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class LvePipeline {
	public: 
		LvePipeline(
			LveDevice& device,
			const std::string& vertFilepath, 
			const std::string& fragFilepath,
			const PipelineConfigInfo& config
		);

 		~LvePipeline();

		LvePipeline(const LvePipeline&) = delete;
		LvePipeline operator=(const LvePipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static PipelineConfigInfo defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, 
			uint32_t width, uint32_t height);
	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(
			const std::string& vertFilepath, 
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		LveDevice& _device;
		VkPipeline _graphicsPipeline;
		VkShaderModule _vertShaderModule;
		VkShaderModule _fragShaderModule;
	};
}