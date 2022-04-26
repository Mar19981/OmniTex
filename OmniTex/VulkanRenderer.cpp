#include "VulkanRenderer.h"

#include <QCoreApplication>

static constexpr int UNIFORM_DATA_SIZE = 3 * 16 * sizeof(float) + sizeof(float);
static constexpr int UNIFORM_FRAG_DATA_SIZE = 4 * 5 * sizeof(float) + 6 * sizeof(float);

static inline VkDeviceSize aligned(VkDeviceSize v, VkDeviceSize byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

VulkanRenderer::VulkanRenderer(VulkanWindow* w, QImage& df, QImage& sp, QImage& dp, QImage& n, QImage& a)
    : window(w), camera(Camera(QVector3D(0, 0, 10))), uv(QVector2D(1.0f, 1.0f)), light(), sphere(std::make_unique<Mesh>(R"(models\sphere.obj)")),
    box(std::make_unique<Mesh>(R"(models\box.obj)")), teapot(std::make_unique<Mesh>(R"(models\teapot.obj)")), cylinder(std::make_unique<Mesh>(R"(models\cylinder.obj)")),
    plane(std::make_unique<Mesh>(R"(models\plane.obj)")), primitiveType(PrimitiveType::SPHERE), isAnimating(false), shininess(150.0f), 
    displacementFactor(1.0f), background(QColor())
{
    diffuse.texture = df;
    specular.texture = sp;
    normal.texture = n;
    displacement.texture = dp;
    ao.texture = a;
    plane->setRotation(90.0f, 0.0f, 0.0f);
}
bool VulkanRenderer::createTexture(Image& image, Image& staging)
{
    image.texture.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    QVulkanFunctions* f = window->vulkanInstance()->functions();
    image.format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties props{};
    f->vkGetPhysicalDeviceFormatProperties(window->physicalDevice(), image.format, &props);
    const bool canSampleLinear = (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    const bool canSampleOptimal = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    if (!canSampleLinear && !canSampleOptimal) {
        return false;
        if (canSampleLinear) {
            if (!createTextureImage(image.texture.size(), image,
                VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT,
                window->hostVisibleMemoryIndex()))
                return false;

            if (!writeLinearImage(image.texture, image))
                return false;

            image.pending = true;
        }
        else {
            if (!createTextureImage(image.texture.size(), staging,
                VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                window->hostVisibleMemoryIndex()))
                return false;

            if (!createTextureImage(image.texture.size(), image,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                window->deviceLocalMemoryIndex()))
                return false;

            if (!writeLinearImage(image.texture, staging))
                return false;

            staging.pending = true;
        }

        VkImageViewCreateInfo viewInfo{};

        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image.img;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = image.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = viewInfo.subresourceRange.layerCount = 1;

        VkResult err = deviceFunctions->vkCreateImageView(dev, &viewInfo, nullptr, &image.view);
        if (err != VK_SUCCESS) {
            qWarning("Failed to create image view for texture!");
            return false;
        }

        image.size = image.texture.size();

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.minFilter = samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeV = samplerInfo.addressModeW = samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.maxAnisotropy = 1.0f;
        err = deviceFunctions->vkCreateSampler(dev, &samplerInfo, nullptr, &image.sampler);

        if (err != VK_SUCCESS)
            throw FatalException("Failed to create sampler!");
        return true;
    }
}
bool VulkanRenderer::createTextureImage(const QSize& size, Image& image,
    VkImageTiling tiling, VkImageUsageFlags usage, uint32_t memIndex)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = image.format;
    imageInfo.extent.width = size.width();
    imageInfo.extent.height = size.height();
    imageInfo.arrayLayers = imageInfo.mipLevels = imageInfo.extent.depth = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    if (deviceFunctions->vkCreateImage(dev, &imageInfo, nullptr, &image.img) != VK_SUCCESS) {
        qWarning("Failed to create linear image for texture!");
        return false;
    }

    VkMemoryRequirements memReq{};
    deviceFunctions->vkGetImageMemoryRequirements(dev, image.img, &memReq);

    if (!(memReq.memoryTypeBits & (1 << memIndex))) {
        VkPhysicalDeviceMemoryProperties physDevMemProps;
        window->vulkanInstance()->functions()->vkGetPhysicalDeviceMemoryProperties(window->physicalDevice(), &physDevMemProps);
        for (uint32_t i = 0; i < physDevMemProps.memoryTypeCount; ++i) {
            if (!(memReq.memoryTypeBits & (1 << i)))
                continue;
            memIndex = i;
        }
    }
    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memReq.size,
        memIndex
    };
    qDebug("allocating %u bytes for texture image", uint32_t(memReq.size));

    if (deviceFunctions->vkAllocateMemory(dev, &allocInfo, nullptr, &image.memory) != VK_SUCCESS) {
        qWarning("Failed to allocate memory for linear image!");
        return false;
    }

    if (deviceFunctions->vkBindImageMemory(dev, image.img, image.memory, 0) != VK_SUCCESS) {
        qWarning("Failed to bind linear image memory!");
        return false;
    }

    return true;
}

bool VulkanRenderer::writeLinearImage(const QImage& img, Image& image)
{
    VkImageSubresource subres = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,         
        0
    };
    VkSubresourceLayout layout;
    deviceFunctions->vkGetImageSubresourceLayout(dev, image.img, &subres, &layout);

     uint8_t* p;
    if (deviceFunctions->vkMapMemory(dev, image.memory, layout.offset, layout.size, 0, reinterpret_cast<void**>(&p))
        != VK_SUCCESS) {
        qWarning("Failed to map memory for linear image!");
        return false;
    }
    for (int y = 0; y < img.height(); ++y) {
        const uint8_t* line = img.constScanLine(y);
        memcpy(p, line, img.width() * 4);
        p += layout.rowPitch;
    }
    deviceFunctions->vkUnmapMemory(dev, image.memory);
    return true;
}

void VulkanRenderer::ensureTexture(Image& image, Image& staging, VkPipelineStageFlagBits stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
{
    if (!image.pending && !staging.pending)
        return;

    Q_ASSERT(image.pending != staging.pending);
        VkCommandBuffer cb = window->currentCommandBuffer();
        VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = barrier.subresourceRange.layerCount = 1;

    if (image.pending) {
        image.pending = false;

        barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.image = image.img;

        deviceFunctions->vkCmdPipelineBarrier(cb,
            VK_PIPELINE_STAGE_HOST_BIT,
            stage,
            0, 0, nullptr, 0, nullptr,
            1, &barrier);
    }
    else {
        staging.pending = false;
        barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.image = staging.img;
        deviceFunctions->vkCmdPipelineBarrier(cb,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr,
            1, &barrier);

        barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.image = image.img;
        deviceFunctions->vkCmdPipelineBarrier(cb,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr,
            1, &barrier);
                VkImageCopy copyInfo{};
        copyInfo.dstSubresource.aspectMask = copyInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyInfo.extent.depth = copyInfo.dstSubresource.layerCount = copyInfo.srcSubresource.layerCount = 1;
        copyInfo.extent.width = image.size.width();
        copyInfo.extent.height = image.size.height();
        deviceFunctions->vkCmdCopyImage(cb, staging.img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.image = image.img;
        deviceFunctions->vkCmdPipelineBarrier(cb,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            stage,
            0, 0, nullptr, 0, nullptr,
            1, &barrier);
    }
}

void VulkanRenderer::allocateModel(std::unique_ptr<Mesh>& model)
{
    if (!model) return;
    const VkPhysicalDeviceLimits* pdevLimits = &window->physicalDeviceProperties()->limits;
    const VkDeviceSize uniAlign = pdevLimits->minUniformBufferOffsetAlignment;
    const VkDeviceSize vertexAllocSize = aligned(sizeof(float) * model->getVerticesCount(), uniAlign);
    VkBufferCreateInfo vertexBufferInfo{};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = vertexAllocSize;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (deviceFunctions->vkCreateBuffer(dev, &vertexBufferInfo, nullptr, &model->getVertexBuffer()) != VK_SUCCESS)
        throw FatalException("Failed to create buffer!");

    VkMemoryRequirements vertexMemReq{};
    deviceFunctions->vkGetBufferMemoryRequirements(dev, model->getVertexBuffer(), &vertexMemReq);

    VkMemoryAllocateInfo vertexMemAllocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        vertexMemReq.size,
        window->hostVisibleMemoryIndex()
    };

    if (deviceFunctions->vkAllocateMemory(dev, &vertexMemAllocInfo, nullptr, &model->getVertexMemory()) != VK_SUCCESS)
        throw FatalException("Failed to allocate memory!");

        if (deviceFunctions->vkBindBufferMemory(dev, model->getVertexBuffer(), model->getVertexMemory(), 0) != VK_SUCCESS)
        throw FatalException("Failed to bind buffer memory!");

     uint8_t* p;
    if (deviceFunctions->vkMapMemory(dev, model->getVertexMemory(), 0, vertexMemReq.size, 0, reinterpret_cast<void**>(&p)) != VK_SUCCESS)
        throw FatalException("Failed to map memory!");
    memcpy(p, model->getVertexData(), sizeof(float) * model->getVerticesCount());
    deviceFunctions->vkUnmapMemory(dev, model->getVertexMemory());

}

void VulkanRenderer::deallocateModel(std::unique_ptr<Mesh>& model)
{
    if (!model) return;
    if (model->getVertexBuffer())
        deviceFunctions->vkDestroyBuffer(dev, model->getVertexBuffer(), nullptr);
    if (model->getVertexMemory())
        deviceFunctions->vkFreeMemory(dev, model->getVertexMemory(), nullptr);
}

void VulkanRenderer::createDescriptorSetLayout()
{
    std::array<VkDescriptorPoolSize, 7> descPoolSizes{};
    descPoolSizes.at(0) =
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t(concurrentFrameCount) };
    descPoolSizes.at(1) =
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(concurrentFrameCount) };
    descPoolSizes.at(2) =
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(concurrentFrameCount) };
    descPoolSizes.at(3) =
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(concurrentFrameCount) };
    descPoolSizes.at(4) =
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(concurrentFrameCount) };
    descPoolSizes.at(5) =
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(concurrentFrameCount) };
    descPoolSizes.at(6) =
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t(concurrentFrameCount) };

    VkDescriptorPoolCreateInfo descPoolInfo{};

    descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descPoolInfo.maxSets = concurrentFrameCount;
    descPoolInfo.poolSizeCount = descPoolSizes.size();
    descPoolInfo.pPoolSizes = descPoolSizes.data();
    auto err = deviceFunctions->vkCreateDescriptorPool(dev, &descPoolInfo, nullptr, &descriptorPool);
    if (err != VK_SUCCESS)
        throw FatalException("Failed to create descriptor pool!");

    std::array<VkDescriptorSetLayoutBinding, 7> layoutBinding{};
    layoutBinding.at(0) =
    {
        0, 
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        nullptr
    };
    layoutBinding.at(1) =
    {
        1, 
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };
    layoutBinding.at(2) =
    {
        2, 
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1, 
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };
    layoutBinding.at(3) =
    {
        3,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1, 
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        nullptr
    };
    layoutBinding.at(4) =
    {
        4, 
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1, 
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };
    layoutBinding.at(5) =
    {
        5, 
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1, 
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    }; 
    layoutBinding.at(6) =
    {
        6, 
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1, 
        VK_SHADER_STAGE_FRAGMENT_BIT  | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_VERTEX_BIT,
        nullptr
    };

    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,         
        nullptr,
        0,         
        layoutBinding.size(),         
        layoutBinding.data()     
    };

    if (deviceFunctions->vkCreateDescriptorSetLayout(dev, &descLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        throw FatalException("Failed to create descriptor set layout!");

}

void VulkanRenderer::createUniformBuffers(VkBuffer& buf, VkDeviceMemory& mem, std::array<VkDescriptorBufferInfo, QVulkanWindow::MAX_CONCURRENT_FRAME_COUNT>& infos, int size)
{
    const VkPhysicalDeviceLimits* pdevLimits = &window->physicalDeviceProperties()->limits;
    const VkDeviceSize uniAlign = pdevLimits->minUniformBufferOffsetAlignment;
    const VkDeviceSize uniformAllocSize = aligned(size, uniAlign);

    VkBufferCreateInfo uniformBuffersInfo{};
    uniformBuffersInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniformBuffersInfo.size = concurrentFrameCount * uniformAllocSize;
    uniformBuffersInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    if (deviceFunctions->vkCreateBuffer(dev, &uniformBuffersInfo, nullptr, &buf) != VK_SUCCESS)
        throw FatalException("Failed to create buffer!");

    VkMemoryRequirements uniformMemReq{};
    deviceFunctions->vkGetBufferMemoryRequirements(dev, buf, &uniformMemReq);
    VkMemoryAllocateInfo uniformMemAllocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        uniformMemReq.size,
        window->hostVisibleMemoryIndex()
    };

    if (deviceFunctions->vkAllocateMemory(dev, &uniformMemAllocInfo, nullptr, &mem) != VK_SUCCESS)
        throw FatalException("Failed to allocate memory!");

    if (deviceFunctions->vkBindBufferMemory(dev, buf, mem, 0) != VK_SUCCESS)
        throw FatalException("Failed to bind buffer memory!");

    for (int i = 0; i < concurrentFrameCount; ++i) {
        const VkDeviceSize offset = i * uniformAllocSize;
        infos.at(i).buffer = buf;
        infos.at(i).offset = offset;
        infos.at(i).range = uniformAllocSize;
    }
}

void VulkanRenderer::createDescriptorSets()
{
        for (int i = 0; i < concurrentFrameCount; ++i) {
            VkDescriptorSetAllocateInfo descSetAllocInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            descriptorPool,
            1,
            &descriptorSetLayout
        };
                if (deviceFunctions->vkAllocateDescriptorSets(dev, &descSetAllocInfo, &descriptorSets.at(i)) != VK_SUCCESS)
            throw FatalException("Failed to allocate descriptor set!");

        std::array<VkWriteDescriptorSet, 7> descWrite{};

        descWrite.at(0).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.at(0).dstSet = descriptorSets.at(i);
        descWrite.at(0).dstBinding = 0;
        descWrite.at(0).descriptorCount = 1;
        descWrite.at(0).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descWrite.at(0).pBufferInfo = &tessEvalUniformBufferInfo.at(i);

        VkDescriptorImageInfo diffuseImageInfo = {
            diffuse.sampler,
            diffuse.view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        descWrite.at(1).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.at(1).dstSet = descriptorSets.at(i);
        descWrite.at(1).dstBinding = 1;
        descWrite.at(1).descriptorCount = 1;
        descWrite.at(1).descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.at(1).pImageInfo = &diffuseImageInfo;

        VkDescriptorImageInfo specularImageInfo = {
            specular.sampler,
            specular.view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        descWrite.at(2).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.at(2).dstSet = descriptorSets.at(i);
        descWrite.at(2).dstBinding = 2;
        descWrite.at(2).descriptorCount = 1;
        descWrite.at(2).descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.at(2).pImageInfo = &specularImageInfo;

        VkDescriptorImageInfo displacementImageInfo = {
            displacement.sampler,
            displacement.view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        descWrite.at(3).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.at(3).dstSet = descriptorSets.at(i);
        descWrite.at(3).dstBinding = 3;
        descWrite.at(3).descriptorCount = 1;
        descWrite.at(3).descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.at(3).pImageInfo = &displacementImageInfo;

        VkDescriptorImageInfo normalImageInfo = {
            normal.sampler,
            normal.view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        descWrite.at(4).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.at(4).dstSet = descriptorSets.at(i);
        descWrite.at(4).dstBinding = 4;
        descWrite.at(4).descriptorCount = 1;
        descWrite.at(4).descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.at(4).pImageInfo = &normalImageInfo;

        VkDescriptorImageInfo aoImageInfo = {
            ao.sampler,
            ao.view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        descWrite.at(5).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.at(5).dstSet = descriptorSets.at(i);
        descWrite.at(5).dstBinding = 5;
        descWrite.at(5).descriptorCount = 1;
        descWrite.at(5).descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrite.at(5).pImageInfo = &aoImageInfo;

        descWrite.at(6).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.at(6).dstSet = descriptorSets.at(i);
        descWrite.at(6).dstBinding = 6;
        descWrite.at(6).descriptorCount = 1;
        descWrite.at(6).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descWrite.at(6).pBufferInfo = &uniformFragmentBufferInfo.at(i);

        deviceFunctions->vkUpdateDescriptorSets(dev, descWrite.size(), descWrite.data(), 0, nullptr);
    }
}
void VulkanRenderer::dealocateTexture(Image& image, Image& staging)
{
    if (image.sampler) {
        deviceFunctions->vkDestroySampler(dev, image.sampler, nullptr);
        image.sampler = VK_NULL_HANDLE;
    }
    if (staging.img) {
        deviceFunctions->vkDestroyImage(dev, staging.img, nullptr);
        staging.img = VK_NULL_HANDLE;
    }
    if (staging.memory) {
        deviceFunctions->vkFreeMemory(dev, staging.memory, nullptr);
        staging.memory = VK_NULL_HANDLE;
    }
    if (image.view) {
        deviceFunctions->vkDestroyImageView(dev, image.view, nullptr);
        image.view = VK_NULL_HANDLE;
    }
    if (image.img) {
        deviceFunctions->vkDestroyImage(dev, image.img, nullptr);
        image.img = VK_NULL_HANDLE;
    }
    if (image.memory) {
        deviceFunctions->vkFreeMemory(dev, image.memory, nullptr);
        image.memory = VK_NULL_HANDLE;
    }
}
void VulkanRenderer::dealocateDescriptorSets()
{
    if (descriptorSetLayout) {
        deviceFunctions->vkDestroyDescriptorSetLayout(dev, descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }
    if (descriptorPool) {
        deviceFunctions->vkDestroyDescriptorPool(dev, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
}
void VulkanRenderer::fillFragmentUniform(int index)
{
    QVector3D lightPosition = camera.getPosition() + light.getOffset();
    uint8_t* data{};
    if (deviceFunctions->vkMapMemory(dev, uniformFragmentMemory, uniformFragmentBufferInfo.at(index).offset, UNIFORM_FRAG_DATA_SIZE, 0, reinterpret_cast<void**>(&data)) != VK_SUCCESS)
        throw FatalException("Failed to map memory");
    float cameraPos[] = { camera.getPosition().x(), camera.getPosition().y(), camera.getPosition().z() },
        lightPos[] = { lightPosition.x(), lightPosition.y(), lightPosition.z() },
        ambientIntensity[] = { light.getAmbient().x(), light.getAmbient().y(), light.getAmbient().z() },
        color[] = { light.getColor().x(), light.getColor().y(), light.getColor().z() },
        specularIntensity[] = { light.getSpecular().x(), light.getSpecular().y(), light.getSpecular().z() },
        attenuation[] = { light.getAttenuation().x(), light.getAttenuation().y(), light.getAttenuation().z() };
        float u = uv.x(), v = uv.y();

    memcpy(data, cameraPos, 3 * sizeof(float));
    data += 4 * sizeof(float);    
    memcpy(data, lightPos, 3 * sizeof(float));
    data += 4 * sizeof(float);    
    memcpy(data, ambientIntensity, 3 * sizeof(float));
    data += 4 * sizeof(float);    
    memcpy(data, color, 3 * sizeof(float));
    data += 4 * sizeof(float);    
    memcpy(data, specularIntensity, 3 * sizeof(float));
    data += 4 * sizeof(float);    
    memcpy(data, attenuation, 3 * sizeof(float));
    data += 3 * sizeof(float);    
    memcpy(data, &u, sizeof(float));
    data += sizeof(float);
    memcpy(data, &v, sizeof(float));
    data += sizeof(float);
    memcpy(data, &shininess, sizeof(float));

    deviceFunctions->vkUnmapMemory(dev, uniformFragmentMemory);
}

void VulkanRenderer::createPipeline()
{
    VkVertexInputBindingDescription vertexBindingDesc = {
    0,     
    14 * sizeof(float),     
    VK_VERTEX_INPUT_RATE_VERTEX
    };
    std::array<VkVertexInputAttributeDescription, 5> vertexAttrDesc{};
    vertexAttrDesc.at(0) =
    {         
        0,
        0,         
        VK_FORMAT_R32G32B32_SFLOAT,         
        0     
    };
    vertexAttrDesc.at(1) =
    {             
        1,
        0,
        VK_FORMAT_R32G32_SFLOAT,
        3 * sizeof(float)
    };
    vertexAttrDesc.at(2) =
    {         
        2,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        5 * sizeof(float)
    };    
    vertexAttrDesc.at(3) =
    {         
        3,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        8 * sizeof(float)
    };    
    vertexAttrDesc.at(4) =
    {         
        4,
        0,
        VK_FORMAT_R32G32B32_SFLOAT,
        11 * sizeof(float)
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = vertexAttrDesc.size();
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc.data();

    VkPipelineCacheCreateInfo pipelineCacheInfo{};
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    if (deviceFunctions->vkCreatePipelineCache(dev, &pipelineCacheInfo, nullptr, &pipelineCache) != VK_SUCCESS)
        throw FatalException("Failed to create pipeline cache!");

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (deviceFunctions->vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw FatalException("Failed to create pipeline layout!");

    Shader vertexShader(R"(shaders\vert.spv)", window->vulkanInstance(), dev, ShaderType::VERTEX),
        fragmentShader(R"(shaders\frag.spv)", window->vulkanInstance(), dev, ShaderType::FRAGMENT),
        tesselationControlShader(R"(shaders\tesc.spv)", window->vulkanInstance(), dev, ShaderType::TESSELLATION_CONTROL),
        tesselationEvaluationShader(R"(shaders\tese.spv)", window->vulkanInstance(), dev, ShaderType::TESSELLATION_EVALUATE);
    auto vertShaderStage = vertexShader.getPipelineShaderStageCreateInfo();
    auto fragShaderStage = fragmentShader.getPipelineShaderStageCreateInfo();
    auto tescShaderStage = tesselationControlShader.getPipelineShaderStageCreateInfo();
    auto teseShaderStage = tesselationEvaluationShader.getPipelineShaderStageCreateInfo();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    std::array<VkPipelineShaderStageCreateInfo, 4> shaderStages = {
        vertShaderStage, tescShaderStage, teseShaderStage, fragShaderStage
    };
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();

    pipelineInfo.pVertexInputState = &vertexInputInfo;

    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    pipelineInfo.pInputAssemblyState = &ia;

    VkPipelineViewportStateCreateInfo vp{};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    pipelineInfo.pViewportState = &vp;

    VkPipelineTessellationStateCreateInfo tessellationStateInfo{};
    tessellationStateInfo.patchControlPoints = 3;
    tessellationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

    VkPipelineRasterizationStateCreateInfo rs{};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.lineWidth = 1.0f;
    pipelineInfo.pRasterizationState = &rs;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &ms;

    VkPipelineDepthStencilStateCreateInfo ds{};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthWriteEnable = ds.depthTestEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineInfo.pDepthStencilState = &ds;

    VkPipelineColorBlendStateCreateInfo cb{};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    VkPipelineColorBlendAttachmentState att{};
    att.colorWriteMask = 0xF;
    att.blendEnable = VK_TRUE;
    att.srcAlphaBlendFactor = att.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    att.dstAlphaBlendFactor = att.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    att.alphaBlendOp = att.colorBlendOp = VK_BLEND_OP_ADD;
    cb.attachmentCount = 1;
    cb.pAttachments = &att;
    pipelineInfo.pColorBlendState = &cb;

    VkDynamicState dynEnable[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn{};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = sizeof(dynEnable) / sizeof(VkDynamicState);
    dyn.pDynamicStates = dynEnable;
    pipelineInfo.pDynamicState = &dyn;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = window->defaultRenderPass();
    pipelineInfo.pTessellationState = &tessellationStateInfo;

    if (deviceFunctions->vkCreateGraphicsPipelines(dev, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw FatalException("Failed to create graphics pipeline!");
}

void VulkanRenderer::initResources()
{
    qDebug("initResources");

    dev = window->device(); 
    deviceFunctions = window->vulkanInstance()->deviceFunctions(dev); 
    concurrentFrameCount = window->concurrentFrameCount(); 
    allocateModel(sphere);
    allocateModel(plane);
    allocateModel(teapot);
    allocateModel(cylinder);
    allocateModel(box);
    changeMesh(PrimitiveType::SPHERE);

    createUniformBuffers(tessEvalUniformBuffer, tessEvalUniformMemory, tessEvalUniformBufferInfo, UNIFORM_DATA_SIZE);
    createUniformBuffers(uniformFragmentBuffer, uniformFragmentMemory, uniformFragmentBufferInfo, UNIFORM_FRAG_DATA_SIZE);

    if (!createTexture(diffuse, diffuseStaging))
        throw FatalException("Failed to create texture");    
    if (!createTexture(specular, specularStaging))
        throw FatalException("Failed to create texture");    
    if (!createTexture(normal, normalStaging))
        throw FatalException("Failed to create texture");    
    if (!createTexture(displacement, displacementStaging))
        throw FatalException("Failed to create texture");    
    if (!createTexture(ao, aoStaging))
        throw FatalException("Failed to create texture");

    createDescriptorSetLayout();
    createDescriptorSets();
    createPipeline();
}

void VulkanRenderer::initSwapChainResources()
{
    qDebug("initSwapChainResources");

    projectionMatrix = window->clipCorrectionMatrix();     const QSize sz = window->swapChainImageSize();
    projectionMatrix.perspective(45.0f, sz.width() / (float)sz.height(), 0.01f, 500.0f);
}

void VulkanRenderer::releaseSwapChainResources()
{
    qDebug("releaseSwapChainResources");
}

void VulkanRenderer::releaseResources()
{
    qDebug("releaseResources");

    dealocateTexture(diffuse, diffuseStaging);
    dealocateTexture(displacement, displacementStaging);
    dealocateTexture(ao, aoStaging);
    dealocateTexture(specular, specularStaging);
    dealocateTexture(normal, normalStaging);

    if (pipeline) {
        deviceFunctions->vkDestroyPipeline(dev, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    if (pipelineLayout) {
        deviceFunctions->vkDestroyPipelineLayout(dev, pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }
    if (pipelineCache) {
        deviceFunctions->vkDestroyPipelineCache(dev, pipelineCache, nullptr);
        pipelineCache = VK_NULL_HANDLE;
    }
    dealocateDescriptorSets();
    currentMesh = nullptr;
    deallocateModel(sphere);
    deallocateModel(plane);
    deallocateModel(cylinder);
    deallocateModel(teapot);
    deallocateModel(box);
    deallocateModel(custom);

    if (tessEvalUniformBuffer) {
        deviceFunctions->vkDestroyBuffer(dev, tessEvalUniformBuffer, nullptr);
        tessEvalUniformBuffer = VK_NULL_HANDLE;
    }

    if (tessEvalUniformMemory) {
        deviceFunctions->vkFreeMemory(dev, tessEvalUniformMemory, nullptr);
        tessEvalUniformMemory = VK_NULL_HANDLE;
    }    
    if (uniformFragmentBuffer) {
        deviceFunctions->vkDestroyBuffer(dev, uniformFragmentBuffer, nullptr);
        uniformFragmentBuffer = VK_NULL_HANDLE;
    }

    if (uniformFragmentMemory) {
        deviceFunctions->vkFreeMemory(dev, uniformFragmentMemory, nullptr);
        uniformFragmentMemory = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::startNextFrame()
{
    const QSize sz = window->swapChainImageSize();

    ensureTexture(diffuse, diffuseStaging);
    ensureTexture(specular, specularStaging);
    ensureTexture(normal, normalStaging);
    ensureTexture(displacement, displacementStaging, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT);
    ensureTexture(ao, aoStaging);

    VkClearColorValue clearColor = { { background.redF(), background.greenF(), background.blueF(), 1 } };
    VkClearDepthStencilValue clearDS = { 1, 0 };
    VkClearValue clearValues[2]{};
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = clearDS;


    VkRenderPassBeginInfo rpBeginInfo{};

    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = window->defaultRenderPass();
    rpBeginInfo.framebuffer = window->currentFramebuffer();
    rpBeginInfo.renderArea.extent.width = sz.width();
    rpBeginInfo.renderArea.extent.height = sz.height();
    rpBeginInfo.clearValueCount = 2;
    rpBeginInfo.pClearValues = clearValues;
    VkCommandBuffer cmdBuf = window->currentCommandBuffer();

    deviceFunctions->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
     uint8_t* p;
    
    if (deviceFunctions->vkMapMemory(dev, tessEvalUniformMemory, tessEvalUniformBufferInfo[window->currentFrame()].offset,
        UNIFORM_DATA_SIZE, 0, reinterpret_cast<void**>(&p)) != VK_SUCCESS)
        throw FatalException("Failed to map memory!");

    QMatrix4x4 m = currentMesh->getModelMatrix();
    QVector3D pos = currentMesh->getPosition();
    m.translate(-pos);
    m.rotate(rotation, 0, 1, 0);
    m.translate(pos);

    memcpy(p, m.constData(), 16 * sizeof(float));
    p += 16 * sizeof(float);
    memcpy(p, camera.getViewMatrix().constData(), 16 * sizeof(float));
    p += 16 * sizeof(float);
    memcpy(p, projectionMatrix.constData(), 16 * sizeof(float));
    p += 16 * sizeof(float);
    memcpy(p, &displacementFactor, sizeof(float));
    deviceFunctions->vkUnmapMemory(dev, tessEvalUniformMemory);
    fillFragmentUniform(window->currentFrame());

     if (isAnimating)
        rotation += 1.0f;

    deviceFunctions->vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    deviceFunctions->vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
        &descriptorSets.at(window->currentFrame()), 0, nullptr);
    VkDeviceSize vbOffset = 0;
    deviceFunctions->vkCmdBindVertexBuffers(cmdBuf, 0, 1, &currentMesh->getVertexBuffer(), &vbOffset);

    VkViewport viewport;
    viewport.x = viewport.y = 0;
    viewport.width = sz.width();
    viewport.height = sz.height();
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    deviceFunctions->vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = viewport.width;
    scissor.extent.height = viewport.height;
    deviceFunctions->vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    deviceFunctions->vkCmdDraw(cmdBuf, currentMesh->getVerticesCount(), 1, 0, 0);

    deviceFunctions->vkCmdEndRenderPass(cmdBuf);
    
    window->frameReady();
    window->requestUpdate();
}

void VulkanRenderer::updateTexture(QImage& img, MapType type)
{
    switch (type) {
        case MapType::DIFFUSE: {
            dealocateTexture(diffuse, diffuseStaging);
            diffuse.texture = img;
            if (!createTexture(diffuse, diffuseStaging))
                throw FatalException("Failed to create texture");
        }; break;
        case MapType::DISPLACEMENT: {
            dealocateTexture(displacement, displacementStaging);
            displacement.texture = img;
            if (!createTexture(displacement, displacementStaging))
                throw FatalException("Failed to create texture");
        }; break;
        case MapType::SPECULAR: {
            dealocateTexture(specular, specularStaging);
            specular.texture = img;
            if (!createTexture(specular, specularStaging))
                throw FatalException("Failed to create texture");
        }; break;
        case MapType::NORMAL: {
            dealocateTexture(normal, normalStaging);
            normal.texture = img;
            if (!createTexture(normal, normalStaging))
                throw FatalException("Failed to create texture");
        }; break;
        case MapType::AO: {
            dealocateTexture(ao, aoStaging);
            ao.texture = img;
            if (!createTexture(ao, aoStaging))
                throw FatalException("Failed to create texture");
        }; break;
    }
    dealocateDescriptorSets();
    createDescriptorSetLayout();
    createDescriptorSets();
}

void VulkanRenderer::changeMesh(PrimitiveType type)
{
    if (primitiveType == PrimitiveType::CUSTOM)
        deallocateModel(custom);
    switch (type) {
        case PrimitiveType::SPHERE: currentMesh = sphere.get(); break;
        case PrimitiveType::BOX: currentMesh = box.get(); break;
        case PrimitiveType::CYLINDER: currentMesh = cylinder.get(); break;
        case PrimitiveType::TEAPOT: currentMesh = teapot.get(); break;
        case PrimitiveType::PLANE: currentMesh = plane.get(); break;
        case PrimitiveType::CUSTOM: currentMesh = custom.get(); break;
    }
    primitiveType = type;

}

void VulkanRenderer::walk(float amount)
{
    camera.walk(amount);
}

void VulkanRenderer::strafe(float amount)
{
    camera.strafe(amount);
}

void VulkanRenderer::pitch(float angle)
{
    camera.pitch(angle);
}

void VulkanRenderer::yaw(float angle)
{
    camera.yaw(angle);
}

void VulkanRenderer::setUV(QVector2D u)
{
    uv = u;
}

void VulkanRenderer::setLightColor(QColor c)
{
    QVector3D lightColor(c.redF(), c.greenF(), c.blueF());
    light.setColor(lightColor);
}

void VulkanRenderer::setBackground(QColor c)
{
    background = c;
}

void VulkanRenderer::loadModel(const std::string& path)
{
    if (custom)
        custom.release();
    custom = std::make_unique<Mesh>(path);
    allocateModel(custom);
    changeMesh(PrimitiveType::CUSTOM);
}

void VulkanRenderer::toogleAnimating()
{
    isAnimating = !isAnimating;
}

void VulkanRenderer::setShininess(float amount)
{
    shininess = amount;
}

void VulkanRenderer::setDisplacementFactor(float amount)
{
    displacementFactor = amount;
}

void VulkanRenderer::setLightIntensity(float amount)
{
    light.setIntensity(amount);
}

void VulkanRenderer::translateLight(float dx, float dy, float dz = 0.0f)
{
    light.translate(dx, dy, dz);
}
