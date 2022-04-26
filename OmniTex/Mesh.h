#pragma once
#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <array>
#include <QVulkanFunctions>
#include "tiny_obj_loader.h"
#include "InvalidModelException.h"


struct Vertex {
	QVector3D position, normal; 	
	QVector2D texCoords; 
};

class Mesh
{
public: 
	Mesh(const std::string_view& modelPath); 		
	VkBuffer& getVertexBuffer() { return vertexBuffer; };
	VkDeviceMemory& getVertexMemory() { return vertexMemory; };
	const float* getVertexData() const { return vertices.data(); };
	uint32_t getVerticesCount() const { return static_cast<uint32_t>(vertices.size()); };
	QMatrix4x4 getModelMatrix() const { return modelMatrix; };
	QVector3D getPosition() { return position; };
	void setPosition(float x, float y, float z);
	void setRotation(float x, float y, float z);

private:
	const std::string PATH; 	
	QMatrix4x4 modelMatrix; 	
	QVector3D position; 	
	std::vector<float> vertices; 	
	VkBuffer vertexBuffer; 	
	VkDeviceMemory vertexMemory; 
	void loadModel(); 
};

