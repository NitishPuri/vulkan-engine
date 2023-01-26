#include "lve_pipeline.h"

#include <fstream>
#include <stdexcept>
#include <iostream>

#include <filesystem>

namespace lve {

	LvePipeline::LvePipeline(LveDevice& device,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const PipelineConfigInfo& config)
		: _device(device)
	{
		createGraphicsPipeline(vertFilepath, fragFilepath, config);
	}

	LvePipeline::~LvePipeline() {

	}

	std::vector<char> LvePipeline::readFile(const std::string& filepath) {
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		if (!std::filesystem::exists(filepath)) {
			throw std::runtime_error("File not found: " + filepath);
		}

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + filepath);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	PipelineConfigInfo LvePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
	{
		PipelineConfigInfo configInfo{};

		return configInfo;
	}

	void LvePipeline::createGraphicsPipeline(
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const PipelineConfigInfo& configInfo)
	{
		auto vertCode = readFile(vertFilepath);
		auto fragCode = readFile(fragFilepath);

		std::cout << "Vertex shader code size: " << vertCode.size() << std::endl;
		std::cout << "Fragment shader code size: " << fragCode.size() << std::endl;
	}

	void LvePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(_device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module.");
		}
	}
}