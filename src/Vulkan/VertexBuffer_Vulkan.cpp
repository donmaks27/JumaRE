// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "VertexBuffer_Vulkan.h"

#include "Material_Vulkan.h"
#include "RenderEngine_Vulkan.h"
#include "RenderOptions_Vulkan.h"
#include "vulkanObjects/VulkanBuffer.h"
#include "vulkanObjects/VulkanCommandBuffer.h"
#include "JumaRE/vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    VertexBuffer_Vulkan::~VertexBuffer_Vulkan()
    {
        clearVulkan();
    }

    bool VertexBuffer_Vulkan::initInternal(const VertexBufferData& data)
    {
        if (data.vertexCount == 0)
        {
            JUTILS_LOG(error, JSTR("Empty vertex buffer data"));
            return false;
        }
        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const RegisteredVertexDescription* description = renderEngine->findVertex(getVertexID());

        VulkanBuffer* vertexBuffer = renderEngine->getVulkanBuffer();
        vertexBuffer->initGPU(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, 
            description->vertexSize * data.vertexCount, data.verticesData
        );
        if (!vertexBuffer->isValid())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize vulkan vertex buffer"));
            renderEngine->returnVulkanBuffer(vertexBuffer);
            return false;
        }

        if (data.indexCount > 0)
        {
            VulkanBuffer* indexBuffer = renderEngine->getVulkanBuffer();
            indexBuffer->initGPU(
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, 
                sizeof(uint32) * data.indexCount, data.indicesData
            );
            if (!indexBuffer->isValid())
            {
                JUTILS_LOG(error, JSTR("Failed to initialize vulkan index buffer"));
                renderEngine->returnVulkanBuffer(vertexBuffer);
                renderEngine->returnVulkanBuffer(indexBuffer);
                return false;
            }

            m_IndexBuffer = indexBuffer;
            m_RenderElementsCount = data.indexCount;
        }
        else
        {
            m_RenderElementsCount = data.vertexCount;
        }

        m_VertexBuffer = vertexBuffer;
        return true;
    }

    void VertexBuffer_Vulkan::clearAsset()
    {
        clearVulkan();
        Super::clearAsset();
    }
    void VertexBuffer_Vulkan::clearVulkan()
    {
        if (m_VertexBuffer != nullptr)
        {
            RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
            renderEngine->returnVulkanBuffer(m_VertexBuffer);
            renderEngine->returnVulkanBuffer(m_IndexBuffer);

            m_VertexBuffer = nullptr;
            m_IndexBuffer = nullptr;
            m_RenderElementsCount = 0;
        }
    }

    void VertexBuffer_Vulkan::render(const RenderOptions* renderOptions, Material* material)
    {
        Material_Vulkan* materialVulan = dynamic_cast<Material_Vulkan*>(material);
        if ((materialVulan == nullptr) || !materialVulan->bindMaterial(renderOptions, this))
        {
            return;
        }

        const RenderOptions_Vulkan* optionsVulkan = reinterpret_cast<const RenderOptions_Vulkan*>(renderOptions);
        VkCommandBuffer commandBuffer = optionsVulkan->commandBuffer->get();

        VkBuffer vertexBuffer = m_VertexBuffer->get();
        constexpr VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        if (m_IndexBuffer == nullptr)
        {
            vkCmdDraw(commandBuffer, m_RenderElementsCount, 1, 0, 0);
        }
        else
        {
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->get(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, m_RenderElementsCount, 1, 0, 0, 0);
        }

        materialVulan->unbindMaterial(renderOptions, this);
    }
}

#endif
