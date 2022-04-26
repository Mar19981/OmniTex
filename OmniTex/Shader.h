
#pragma once
#include <qvulkaninstance.h>
#include <qvulkanfunctions.h>
#include <vector>
#include <fstream>
#include "ShaderType.h"
#include "FatalException.h"

class Shader
{
public:
	Shader(const std::string_view& path, QVulkanInstance* instance, VkDevice dev, ShaderType type);
	~Shader(); 		VkPipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo();
	VkShaderModule getShaderModule() { return shaderModule; };
private:
	std::vector<char> code; 	
	VkShaderModule shaderModule; 	
	VkPipelineShaderStageCreateInfo pipelineStageCreateInfo; 	
	QVulkanInstance* instance; 	
	VkDevice dev; 		
	void loadShader(const std::string_view& path);
	void setShaderType(const ShaderType type);
	void createShaderModule();
};

