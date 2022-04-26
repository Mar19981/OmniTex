#pragma once
#include <QImage>
#include <QVulkanFunctions>
struct Image {
	QImage texture; 	
	VkImage img{ VK_NULL_HANDLE }; 
	VkImageView view{ VK_NULL_HANDLE }; 	
	VkDeviceMemory memory{ VK_NULL_HANDLE }; 	
	VkSampler sampler{ VK_NULL_HANDLE }; 	
	QSize size{}; 	
	VkFormat format; 	
	bool pending{}; 
};