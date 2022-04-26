#include "Shader.h"

Shader::Shader(const std::string_view& path, QVulkanInstance* inst, VkDevice device, ShaderType type) : code({}), instance(inst), dev(device), shaderModule{VK_NULL_HANDLE}, pipelineStageCreateInfo({})
{
	pipelineStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 	
	pipelineStageCreateInfo.pName = "main"; 	
	setShaderType(type);
	loadShader(path);
	createShaderModule();
}

Shader::~Shader()
{
	instance->deviceFunctions(dev)->vkDestroyShaderModule(dev, shaderModule, nullptr); 
}

VkPipelineShaderStageCreateInfo Shader::getPipelineShaderStageCreateInfo()
{
	if (shaderModule == VK_NULL_HANDLE) throw FatalException("Failed to create shader module!");
	return pipelineStageCreateInfo;
}

void Shader::loadShader(const std::string_view& path)
{
	std::ifstream input(path.data(), std::ios_base::binary | std::ios_base::ate);
	if (!input)
		throw std::runtime_error("Failed to open a file '" + std::string(path) + "'!\n");

	code.clear();
	int file_size = static_cast<int>(input.tellg());
	code.resize(file_size);
	input.seekg(std::ios_base::beg);
	input.read(code.data(), file_size);
	input.close();
}

void Shader::setShaderType(const ShaderType type)
{
	switch (type) {
		case ShaderType::FRAGMENT: pipelineStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
		case ShaderType::VERTEX: pipelineStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
		case ShaderType::COMPUTE: pipelineStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
		case ShaderType::TESSELLATION_CONTROL: pipelineStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
		case ShaderType::TESSELLATION_EVALUATE: pipelineStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
		case ShaderType::GEOMETRY: pipelineStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
	}
}

void Shader::createShaderModule()
{
	VkShaderModuleCreateInfo shaderInfo{}; 	
	shaderInfo.codeSize = code.size();  	
	shaderInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); 	
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; 		
	if (instance->deviceFunctions(dev)->vkCreateShaderModule(dev, &shaderInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw FatalException("Failed to create shader module!");
	pipelineStageCreateInfo.module = shaderModule; 
}
