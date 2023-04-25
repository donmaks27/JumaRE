// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "Material_Vulkan.h"

#include "RenderEngine_Vulkan.h"
#include "RenderOptions_Vulkan.h"
#include "RenderTarget_Vulkan.h"
#include "Shader_Vulkan.h"
#include "Texture_Vulkan.h"
#include "VertexBuffer_Vulkan.h"
#include "vulkanObjects/VulkanBuffer.h"
#include "vulkanObjects/VulkanCommandBuffer.h"
#include "vulkanObjects/VulkanImage.h"

namespace JumaRenderEngine
{
    Material_Vulkan::~Material_Vulkan()
    {
        clearVulkan();
    }

    bool Material_Vulkan::initInternal()
    {
        if (getShader()->getUniforms().isEmpty())
        {
            m_MaterialCreated = true;
            return true;
        }

        m_CreateTaskActive = true;
        jasync_task* task = new MaterialCreateTask(this);
        if (!getRenderEngine()->getAsyncAssetTaksQueue().addTask(task))
        {
            JUTILS_LOG(error, JSTR("Failed to start async material creation"));
            delete task;
            clearVulkan();
            return false;
        }
        return true;
    }
    bool Material_Vulkan::createDescriptorSet()
    {
        const Shader_Vulkan* shader = getShader<Shader_Vulkan>();
        const jmap<jstringID, ShaderUniform>& uniforms = shader->getUniforms();
        const uint32 bufferUniformCount = static_cast<uint32>(shader->getUniformBufferDescriptions().getSize());
        uint32 imageUniformCount = 0;
        for (const auto& uniform : uniforms.values())
        {
            switch (uniform.type)
            {
            case ShaderUniformType::Texture: 
                imageUniformCount++;
                break;
            default: ;
            }
        }

        uint8 poolSizeCount = 0;
        VkDescriptorPoolSize poolSizes[2];
        if (bufferUniformCount > 0)
        {
            VkDescriptorPoolSize& poolSize = poolSizes[poolSizeCount++];
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSize.descriptorCount = bufferUniformCount;
        }
        if (imageUniformCount > 0)
        {
            VkDescriptorPoolSize& poolSize = poolSizes[poolSizeCount++];
            poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = imageUniformCount;
        }
        if (poolSizeCount == 0)
        {
            return true;
        }

        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizeCount;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = 1;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool);
        if (result != VK_SUCCESS)
        {
            return false;
        }

        VkDescriptorSetLayout descriptorSetLayout = shader->getDescriptorSetLayout();
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = m_DescriptorPool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &descriptorSetLayout;
        result = vkAllocateDescriptorSets(device, &allocateInfo, &m_DescriptorSet);
        if (result != VK_SUCCESS)
        {
            vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
            m_DescriptorPool = nullptr;
            return false;
        }
        if (!initDescriptorSetData())
        {
            vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
            m_DescriptorSet = nullptr;
            m_DescriptorPool = nullptr;
            return false;
        }
        return true;
    }
    bool Material_Vulkan::initDescriptorSetData()
    {
        if (m_DescriptorSet == nullptr)
        {
            return true;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const jmap<uint32, ShaderUniformBufferDescription>& uniformBufferDescriptions = getShader()->getUniformBufferDescriptions();
        const jmap<jstringID, ShaderUniform>& uniforms = getShader()->getUniforms();

        jarray<VkDescriptorBufferInfo> bufferInfos;
        jarray<VkDescriptorImageInfo> imageInfos;
        jarray<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(static_cast<int32>(uniforms.getSize()));
        if (!uniformBufferDescriptions.isEmpty())
        {
            bufferInfos.reserve(static_cast<int32>(uniformBufferDescriptions.getSize()));
            for (const auto& [uniformLocation, bufferDescription] : uniformBufferDescriptions)
            {
                VulkanBuffer* buffer = renderEngine->getVulkanBuffer();
                if (!buffer->initAccessedGPU(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, { VulkanQueueType::Graphics }, bufferDescription.size))
                {
                    renderEngine->returnVulkanBuffer(buffer);
                    return false;
                }
                m_UniformBuffers.add(uniformLocation, buffer);

                VkDescriptorBufferInfo& bufferInfo = bufferInfos.addDefault();
                bufferInfo.buffer = buffer->get();
                bufferInfo.offset = 0;
                bufferInfo.range = VK_WHOLE_SIZE;
                VkWriteDescriptorSet& descriptorWrite = descriptorWrites.addDefault();
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_DescriptorSet;
                descriptorWrite.dstBinding = uniformLocation;
                descriptorWrite.dstArrayElement = 0;
            }
        }
        if (!uniforms.isEmpty())
        {
            const Texture_Vulkan* defaultTexture = dynamic_cast<const Texture_Vulkan*>(renderEngine->getDefaultTexture());
            VkImageView defaultImageView = defaultTexture->getVulkanImage()->getImageView();
            imageInfos.reserve(static_cast<int32>(uniforms.getSize()));
            for (const auto& uniform : uniforms.values())
            {
                if (uniform.type != ShaderUniformType::Texture)
                {
                    continue;
                }

                VkDescriptorImageInfo& imageInfo = imageInfos.addDefault();
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = defaultImageView;
                imageInfo.sampler = renderEngine->getTextureSampler(defaultTexture->getSamplerType());
                VkWriteDescriptorSet& descriptorWrite = descriptorWrites.addDefault();
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_DescriptorSet;
                descriptorWrite.dstBinding = uniform.shaderLocation;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfo;
            }
        }
        if (!descriptorWrites.isEmpty())
        {
            vkUpdateDescriptorSets(renderEngine->getDevice(), 
                static_cast<uint32>(descriptorWrites.getSize()), descriptorWrites.getData(),
                0, nullptr
            );
        }
        return true;
    }

    bool Material_Vulkan::updateDescriptorSetData()
    {
        if (m_DescriptorSet == nullptr)
        {
            return true;
        }

        const jset<jstringID>& notUpdatedParams = getNotUpdatedParams();
        if (notUpdatedParams.isEmpty())
        {
            return true;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const Texture_Vulkan* defaultTexture = dynamic_cast<const Texture_Vulkan*>(renderEngine->getDefaultTexture());
        const jmap<jstringID, ShaderUniform>& uniforms = getShader()->getUniforms();
        const MaterialParamsStorage& params = getMaterialParams();

        jarray<VkDescriptorImageInfo> imageInfos;
        jarray<VkWriteDescriptorSet> descriptorWrites;
        imageInfos.reserve(static_cast<int32>(uniforms.getSize()));
        descriptorWrites.reserve(static_cast<int32>(uniforms.getSize()));
        for (const auto& paramName : notUpdatedParams)
        {
            const ShaderUniform* uniformPtr = uniforms.find(paramName);
            if (uniformPtr == nullptr)
            {
                continue;
            }
            const ShaderUniform& uniform = *uniformPtr;
            switch (uniform.type)
            {
            case ShaderUniformType::Float:
                {
                    ShaderUniformInfo<ShaderUniformType::Float>::value_type value;
                    if (!params.getValue<ShaderUniformType::Float>(paramName, value))
                    {
                        continue;
                    }
                    VulkanBuffer* buffer = m_UniformBuffers[uniform.shaderLocation];
                    buffer->initMappedData();
                    buffer->setMappedData(&value, sizeof(value), uniform.shaderBlockOffset);
                }
                break;
            case ShaderUniformType::Vec2:
                {
                    ShaderUniformInfo<ShaderUniformType::Vec2>::value_type value;
                    if (!params.getValue<ShaderUniformType::Vec2>(paramName, value))
                    {
                        continue;
                    }
                    VulkanBuffer* buffer = m_UniformBuffers[uniform.shaderLocation];
                    buffer->initMappedData();
                    buffer->setMappedData(&value, sizeof(value), uniform.shaderBlockOffset);
                }
                break;
            case ShaderUniformType::Vec4:
                {
                    ShaderUniformInfo<ShaderUniformType::Vec4>::value_type value;
                    if (!params.getValue<ShaderUniformType::Vec4>(paramName, value))
                    {
                        continue;
                    }
                    VulkanBuffer* buffer = m_UniformBuffers[uniform.shaderLocation];
                    buffer->initMappedData();
                    buffer->setMappedData(&value, sizeof(value), uniform.shaderBlockOffset);
                }
                break;
            case ShaderUniformType::Mat4:
                {
                    ShaderUniformInfo<ShaderUniformType::Mat4>::value_type value;
                    if (!params.getValue<ShaderUniformType::Mat4>(paramName, value))
                    {
                        continue;
                    }
                    VulkanBuffer* buffer = m_UniformBuffers[uniform.shaderLocation];
                    buffer->initMappedData();
                    buffer->setMappedData(&value, sizeof(value), uniform.shaderBlockOffset);
                }
                break;

            case ShaderUniformType::Texture:
                {
                    ShaderUniformInfo<ShaderUniformType::Texture>::value_type value;
                    if (!params.getValue<ShaderUniformType::Texture>(paramName, value))
                    {
                        continue;
                    }
                    VulkanImage* vulkanImage = nullptr;
                    {
                        const Texture_Vulkan* texture = dynamic_cast<Texture_Vulkan*>(value);
                        if (texture != nullptr)
                        {
                            vulkanImage = texture->getVulkanImage();
                        }
                        else
                        {
                            const RenderTarget_Vulkan* renderTarget = dynamic_cast<RenderTarget_Vulkan*>(value);
                            if (renderTarget != nullptr)
                            {
                                vulkanImage = renderTarget->getResultImage();
                            }
                            else if (defaultTexture != nullptr)
                            {
                                vulkanImage = defaultTexture->getVulkanImage();
                            }
                            else
                            {
                                throw std::exception("Invalid default texture");
                            }
                        }
                    }
                    if (vulkanImage == nullptr)
                    {
                        JUTILS_LOG(error, JSTR("Failed to get vulkan image"));
                        continue;
                    }

                    VkDescriptorImageInfo& imageInfo = imageInfos.addDefault();
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = vulkanImage->getImageView();
                    imageInfo.sampler = renderEngine->getTextureSampler(value != nullptr ? value->getSamplerType() : TextureSamplerType());
                    VkWriteDescriptorSet& descriptorWrite = descriptorWrites.addDefault();
                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = m_DescriptorSet;
                    descriptorWrite.dstBinding = uniform.shaderLocation;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pImageInfo = &imageInfo;
                }
                break;

            default: ;
            }
        }
        if (!descriptorWrites.isEmpty())
        {
            vkUpdateDescriptorSets(renderEngine->getDevice(), 
               static_cast<uint32>(descriptorWrites.getSize()), descriptorWrites.getData(),
               0, nullptr
            );
        }
        clearParamsForUpdate();

        if (!m_UniformBuffers.isEmpty())
        {
            // TODO: Put it into one command buffer
            bool needToWait = false;
            for (const auto& buffer : m_UniformBuffers.values())
            {
                needToWait |= buffer->flushMappedData(false);
            }
            if (needToWait)
            {
                vkQueueWaitIdle(renderEngine->getQueue(VulkanQueueType::Transfer)->queue);
            }
        }
        return true;
    }

    void Material_Vulkan::onClearAsset()
    {
        clearVulkan();
        Super::onClearAsset();
    }
    void Material_Vulkan::clearVulkan()
    {
        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        if (m_DescriptorPool != nullptr)
        {
            vkDestroyDescriptorPool(renderEngine->getDevice(), m_DescriptorPool, nullptr);
            m_DescriptorSet = nullptr;
            m_DescriptorPool = nullptr;
        }

        if (!m_UniformBuffers.isEmpty())
        {
            for (const auto& buffer : m_UniformBuffers.values())
            {
                renderEngine->returnVulkanBuffer(buffer);
            }
            m_UniformBuffers.clear();
        }
    }

    bool Material_Vulkan::bindMaterial(const RenderOptions* renderOptions, VertexBuffer_Vulkan* vertexBuffer)
    {
        Shader_Vulkan* shader = getShader<Shader_Vulkan>();
        if (!m_MaterialCreated)
        {
            if (m_CreateTaskActive)
            {
                return false;
            }
            m_MaterialValid = shader->getUniforms().isEmpty() || (m_DescriptorSet != nullptr);
            m_MaterialCreated = true;
        }
        if (!m_MaterialValid)
        {
            return false;
        }

        const RenderOptions_Vulkan* options = reinterpret_cast<const RenderOptions_Vulkan*>(renderOptions);
        MaterialProperties materialProperties = getMaterialProperties();
        materialProperties.depthEnabled &= renderOptions->renderStageProperties.depthEnabled;

        VkCommandBuffer commandBuffer = options->commandBuffer->get();
        return shader->bindRenderPipeline(commandBuffer, vertexBuffer->getVertexID(), options->renderPass, materialProperties)
            && bindDescriptorSet(commandBuffer);
    }

    bool Material_Vulkan::bindDescriptorSet(VkCommandBuffer commandBuffer)
    {
        if (!updateDescriptorSetData())
        {
            return false;
        }

        if (m_DescriptorSet != nullptr)
        {
            const Shader_Vulkan* shader = getShader<Shader_Vulkan>();
            vkCmdBindDescriptorSets(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, shader->getPipelineLayout(), 
                0, 1, &m_DescriptorSet, 0, nullptr
            );
        }
        return true;
    }
}

#endif
