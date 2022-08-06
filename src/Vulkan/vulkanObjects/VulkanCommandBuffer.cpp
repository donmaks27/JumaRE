// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "VulkanCommandBuffer.h"

#include "VulkanCommandPool.h"
#include "../RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    bool VulkanCommandBuffer::submit(const bool waitForFinish)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        return submit(submitInfo, nullptr, waitForFinish);
    }
    bool VulkanCommandBuffer::submit(VkSubmitInfo submitInfo, VkFence fenceOnFinish, const bool waitForFinish)
    {
        const VulkanQueueDescription* queueDescription = m_CommandPool->getRenderEngine<RenderEngine_Vulkan>()->getQueue(m_CommandPool->getQueueType());
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffer;
        const VkResult result = vkQueueSubmit(queueDescription->queue, 1, &submitInfo, fenceOnFinish);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to submit command buffer"));
            return false;
        }

        if (waitForFinish)
        {
            vkQueueWaitIdle(queueDescription->queue);
        }
        return true;
    }

    void VulkanCommandBuffer::returnToCommandPool()
    {
        m_CommandPool->returnCommandBuffer(this);
    }
}

#endif
