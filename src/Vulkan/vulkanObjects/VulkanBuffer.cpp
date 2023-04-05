﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "VulkanBuffer.h"

#include "VulkanCommandPool.h"
#include "../RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    VulkanBuffer::~VulkanBuffer()
    {
        clearVulkan();
    }

    bool VulkanBuffer::initStaging(const uint32 size)
    {
        if (isValid())
        {
            return false;
        }
        if (size == 0)
        {
            return false;
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices = nullptr;
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        const VkResult result = vmaCreateBuffer(getRenderEngine<RenderEngine_Vulkan>()->getAllocator(), &bufferInfo, &allocationInfo, &m_Buffer, &m_Allocation, nullptr);
        if (result != VK_SUCCESS)
        {
            return false;
        }

        m_BufferSize = size;
        m_Mapable = true;
        markAsInitialized();
        return true;
    }
    bool VulkanBuffer::initGPU(const VkBufferUsageFlags usage, const std::initializer_list<VulkanQueueType> accessedQueues, const uint32 size, 
        const void* data)
    {
        if (isValid())
        {
            return false;
        }
        if (size == 0)
        {
            return false;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        jarray<uint32> accessedQueueFamilies;
        accessedQueueFamilies.reserve(accessedQueues.size() + 1);
        accessedQueueFamilies.add(renderEngine->getQueue(VulkanQueueType::Transfer)->familyIndex);
        for (const auto& queue : accessedQueues)
        {
            accessedQueueFamilies.addUnique(renderEngine->getQueue(queue)->familyIndex);
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (accessedQueueFamilies.getSize() > 1)
        {
            bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            bufferInfo.queueFamilyIndexCount = static_cast<uint32>(accessedQueueFamilies.getSize());
            bufferInfo.pQueueFamilyIndices = accessedQueueFamilies.getData();
        }
        else
        {
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferInfo.queueFamilyIndexCount = 0;
            bufferInfo.pQueueFamilyIndices = nullptr;
        }
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        const VkResult result = vmaCreateBuffer(renderEngine->getAllocator(), &bufferInfo, &allocationInfo, &m_Buffer, &m_Allocation, nullptr);
        if (result != VK_SUCCESS)
        {
            return false;
        }

        m_BufferSize = size;
        m_Mapable = false;

        VulkanBuffer* stagingBuffer = renderEngine->getVulkanBuffer();
        if (!stagingBuffer->initStaging(size) || !stagingBuffer->setData(data, size, 0, true) || !stagingBuffer->copyData(this, true))
        {
            renderEngine->returnVulkanBuffer(stagingBuffer);
            clearVulkan();
            return false;
        }

        renderEngine->returnVulkanBuffer(stagingBuffer);
        markAsInitialized();
        return true;
    }
    bool VulkanBuffer::initAccessedGPU(const VkBufferUsageFlags usage, const std::initializer_list<VulkanQueueType> accessedQueues, const uint32 size)
    {
        if (isValid())
        {
            return false;
        }
        if (size == 0)
        {
            return false;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        jarray<uint32> accessedQueueFamilies = { renderEngine->getQueue(VulkanQueueType::Transfer)->familyIndex };
        for (const auto& queue : accessedQueues)
        {
            accessedQueueFamilies.addUnique(renderEngine->getQueue(queue)->familyIndex);
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (accessedQueueFamilies.getSize() > 1)
        {
            bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            bufferInfo.queueFamilyIndexCount = static_cast<uint32>(accessedQueueFamilies.getSize());
            bufferInfo.pQueueFamilyIndices = accessedQueueFamilies.getData();
        }
        else
        {
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferInfo.queueFamilyIndexCount = 0;
            bufferInfo.pQueueFamilyIndices = nullptr;
        }
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
        allocationInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        const VkResult result = vmaCreateBuffer(renderEngine->getAllocator(), &bufferInfo, &allocationInfo, &m_Buffer, &m_Allocation, nullptr);
        if (result != VK_SUCCESS)
        {
            return false;
        }

        VkMemoryPropertyFlags memPropFlags;
        vmaGetAllocationMemoryProperties(renderEngine->getAllocator(), m_Allocation, &memPropFlags);
        if (!(memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        {
            VulkanBuffer* stagingBuffer = renderEngine->getVulkanBuffer();
            if (!stagingBuffer->initStaging(size))
            {
                renderEngine->returnVulkanBuffer(stagingBuffer);
                clearVulkan();
                return false;
            }
            m_StagingBuffer = stagingBuffer;
            m_Mapable = false;
        }
        else
        {
            m_Mapable = true;
        }
        
        m_BufferSize = size;
        markAsInitialized();
        return true;
    }

    void VulkanBuffer::clearVulkan()
    {
        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        m_MappedData = nullptr;
        m_Mapable = false;
        if (m_StagingBuffer != nullptr)
        {
            renderEngine->returnVulkanBuffer(m_StagingBuffer);
            m_StagingBuffer = nullptr;
        }
        if (m_Buffer != nullptr)
        {
            vmaDestroyBuffer(renderEngine->getAllocator(), m_Buffer, m_Allocation);
            m_Buffer = nullptr;
            m_Allocation = nullptr;
        }
        m_BufferSize = 0;
    }

    bool VulkanBuffer::initMappedData()
    {
        if (!isValid())
        {
            return false;
        }

        if (m_StagingBuffer != nullptr)
        {
            if (!m_StagingBuffer->initMappedData())
            {
                return false;
            }
            return true;
        }
        if (!m_Mapable)
        {
            return false;
        }

        if (m_MappedData == nullptr)
        {
            void* data;
            const VkResult result = vmaMapMemory(getRenderEngine<RenderEngine_Vulkan>()->getAllocator(), m_Allocation, &data);
            if (result != VK_SUCCESS)
            {
                return false;
            }
            m_MappedData = data;
        }
        return true;
    }
    bool VulkanBuffer::setMappedData(const void* data, const uint32 size, const uint32 offset)
    {
        if (!isValid())
        {
            return false;
        }
        if ((data == nullptr) || (size == 0) || ((offset + size) > m_BufferSize))
        {
            return false;
        }

        if (m_StagingBuffer != nullptr)
        {
            if (!m_StagingBuffer->setMappedData(data, size, offset))
            {
                return false;
            }
            return true;
        }

        if (m_MappedData == nullptr)
        {
            return false;
        }
        std::memcpy(static_cast<uint8*>(m_MappedData) + offset, data, size);
        return true;
    }
    bool VulkanBuffer::flushMappedData(const bool waitForFinish)
    {
        if (!isValid())
        {
            return false;
        }

        if (m_StagingBuffer != nullptr)
        {
            return m_StagingBuffer->flushMappedData(false) && m_StagingBuffer->copyData(this, waitForFinish);
        }
        if (m_MappedData == nullptr)
        {
            return false;
        }
        vmaUnmapMemory(getRenderEngine<RenderEngine_Vulkan>()->getAllocator(), m_Allocation);
        m_MappedData = nullptr;
        return true;
    }

    bool VulkanBuffer::setData(const void* data, const uint32 size, const uint32 offset, const bool waitForFinish)
    {
        if (!isValid())
        {
            return false;
        }
        if ((data == nullptr) || (size == 0) || ((offset + size) > m_BufferSize))
        {
            return false;
        }
        if (m_StagingBuffer != nullptr)
        {
            return m_StagingBuffer->setDataInternal(data, size, offset) && m_StagingBuffer->copyData(this, waitForFinish);
        }
        return setDataInternal(data, size, offset);
    }
    bool VulkanBuffer::setDataInternal(const void* data, const uint32 size, const uint32 offset)
    {
        if (!m_Mapable)
        {
            return false;
        }
        
        VmaAllocator allocator = getRenderEngine<RenderEngine_Vulkan>()->getAllocator();
        void* mappedData;
        if (vmaMapMemory(allocator, m_Allocation, &mappedData) != VK_SUCCESS)
        {
            return false;
        }
        std::memcpy(static_cast<uint8*>(mappedData) + offset, data, size);
        vmaUnmapMemory(allocator, m_Allocation);
        return true;
    }

    bool VulkanBuffer::copyData(const VulkanBuffer* destinationBuffer, const bool waitForFinish)
    {
        VulkanCommandPool* commandPool = getRenderEngine<RenderEngine_Vulkan>()->getCommandPool(VulkanQueueType::Transfer);
        VulkanCommandBuffer* commandBuffer = commandPool != nullptr ? commandPool->getCommandBuffer() : nullptr;
        if (commandBuffer == nullptr)
        {
            return false;
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer->get(), &beginInfo);

        VkBufferCopy copyRegion;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = m_BufferSize;
        vkCmdCopyBuffer(commandBuffer->get(), m_Buffer, destinationBuffer->get(), 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer->get());

        const bool success = commandBuffer->submit(waitForFinish);
        commandBuffer->returnToCommandPool();
        if (!success)
        {
            return false;
        }
        return true;
    }
}

#endif
