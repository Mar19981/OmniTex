#pragma once
#include <QVulkanWindow>
#include "VulkanWindow.h"
#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"
#include "PrimitiveType.h"
#include "MapType.h"
#include "Light.h"
#include "Image.h"
#include "FatalException.h"



class VulkanRenderer: public QVulkanWindowRenderer
{
public:
    VulkanRenderer(VulkanWindow* w, QImage& df, QImage& sp, QImage& dp, QImage& n, QImage& a);

    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;

    
    void updateTexture(QImage& img, MapType type);     
    void changeMesh(PrimitiveType type);     
    void walk(float amount);     
    void strafe(float amount);     
    void pitch(float angle);     
    void yaw(float angle);     
    void loadModel(const std::string& path);         
    void setUV(QVector2D u);
    void setLightColor(QColor c);
    void setBackground(QColor c);
    void toogleAnimating();
    void setShininess(float amount);
    void setDisplacementFactor(float amount);
    void setLightIntensity(float amount);
    void translateLight(float dx, float dy, float dz);         
    PrimitiveType getMeshType() { return primitiveType; };

private:
    bool createTexture(Image& image, Image& staging);    
    bool createTextureImage(const QSize& size, Image& image,         
        VkImageTiling tiling, VkImageUsageFlags usage, uint32_t memIndex);
    bool writeLinearImage(const QImage& img, Image& image);     
    void ensureTexture(Image& image, Image& staging, VkPipelineStageFlagBits stage);     
    void allocateModel(std::unique_ptr<Mesh>& model);     
    void deallocateModel(std::unique_ptr<Mesh>& model);     
    void createDescriptorSetLayout();     
    void createUniformBuffers(VkBuffer& buf, VkDeviceMemory& mem, std::array<VkDescriptorBufferInfo, QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT>& infos, int size);      
    void createDescriptorSets();      
    void dealocateTexture(Image& image, Image& staging);    
    void dealocateDescriptorSets();     
    void fillFragmentUniform(int index);     
    void createPipeline(); 

    QVulkanWindow* window;     
    QVulkanDeviceFunctions* deviceFunctions;     
    VkDevice dev;         
    std::array<VkDescriptorBufferInfo, QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT> tessEvalUniformBufferInfo{};
    std::array<VkDescriptorBufferInfo, QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT> uniformFragmentBufferInfo{};
    int concurrentFrameCount{}; 
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;     
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;     
    std::array <VkDescriptorSet, QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT> descriptorSets; 
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;     
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;     
    VkPipeline pipeline = VK_NULL_HANDLE; 
    Image diffuse, diffuseStaging, specular, specularStaging, displacement, displacementStaging, normal, normalStaging, ao, aoStaging;     
    QMatrix4x4 projectionMatrix;     
    VkBuffer tessEvalUniformBuffer, uniformFragmentBuffer;     
    VkDeviceMemory tessEvalUniformMemory, uniformFragmentMemory;     
    Camera camera;     
    QVector2D uv; 
    std::unique_ptr<Mesh> sphere, plane, teapot, cylinder, box, custom;     
    Mesh* currentMesh;     
    Light light;     
    PrimitiveType primitiveType;     
    QColor background;     
    bool isAnimating;     
    float rotation{}, displacementFactor, shininess; 
};

