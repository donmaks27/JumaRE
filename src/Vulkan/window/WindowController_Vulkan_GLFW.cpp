// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN) && defined(GLFW_ENABLED)

#include "WindowController_Vulkan_GLFW.h"

#include "../RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    WindowController_Vulkan_GLFW::~WindowController_Vulkan_GLFW()
    {
        clearData_Vulkan_GLFW();
    }

    jarray<const char*> WindowController_Vulkan_GLFW::getVulkanInstanceExtensions() const
    {
        uint32 extensionsCount = 0;
        const char** extenstions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        if (extensionsCount == 0)
        {
            return {};
        }

        jarray<const char*> result(static_cast<int32>(extensionsCount));
        for (int32 index = 0; index < result.getSize(); index++)
        {
            result[index] = extenstions[index];
        }
        return result;
    }

    void WindowController_Vulkan_GLFW::clearData_Vulkan_GLFW()
    {
        if (!m_Windows.isEmpty())
        {
            for (auto& window : m_Windows.values())
            {
                clearWindowDataInternal(&window);
            }
            m_Windows.clear();
        }
    }

    WindowData* WindowController_Vulkan_GLFW::createWindowInternal(const window_id windowID, const WindowCreateInfo& createInfo)
    {
        WindowDataType* windowData = &m_Windows.add(windowID);
        if (!createWindowGLFW(windowID, windowData, createInfo.size, createInfo.title))
        {
            m_Windows.remove(windowID);
            return nullptr;
        }

        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const VkResult result = glfwCreateWindowSurface(
            renderEngine->getVulkanInstance(), windowData->windowGLFW, nullptr, &windowData->vulkanSurface
        );
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create surface for window {}"), windowID);
            destroyWindowInternal(windowID, windowData);
            return nullptr;
        }
        if (!createWindowSwapchain(windowID, windowData))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create swapchain for window {}"), windowID);
            destroyWindowInternal(windowID, windowData);
            return nullptr;
        }
        return windowData;
    }
    void WindowController_Vulkan_GLFW::destroyWindowInternal(const window_id windowID, WindowData* windowData)
    {
        clearWindowDataInternal(windowData);

        m_Windows.remove(windowID);
    }
}

#endif
