// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_Vulkan_GLFW.h"

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include "../RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    WindowController_Vulkan_GLFW::~WindowController_Vulkan_GLFW()
    {
        clearGLFW();
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

    bool WindowController_Vulkan_GLFW::initWindowController()
    {
        if (!Super::initWindowController())
        {
            return false;
        }
        if (!GLFW_init(RenderAPI::Vulkan))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize GLFW"));
            return false;
        }
        return true;
    }

    void WindowController_Vulkan_GLFW::clearGLFW()
    {
        if (!m_Windows.isEmpty())
        {
            for (auto& window : m_Windows)
            {
                clearWindowDataGLFW(window.key, window.value);
            }
            m_Windows.clear();
        }

        GLFW_terminate();
    }

    WindowData* WindowController_Vulkan_GLFW::createWindowInternal(const window_id windowID, const WindowInitProperties& properties)
    {
        if (windowID == window_id_INVALID)
        {
            JUTILS_LOG(error, JSTR("Invalid window ID"));
            return nullptr;
        }
        if (m_Windows.contains(windowID))
        {
            JUTILS_LOG(error, JSTR("Window {} already created"), windowID);
            return nullptr;
        }

        WindowData_Vulkan_GLFW* windowData = &m_Windows.add(windowID);
        if (!GLFW_createWindow(windowData, properties.size, properties.title))
        {
            JUTILS_LOG(error, JSTR("Failed to create window {}"), windowID);
            m_Windows.remove(windowID);
            return nullptr;
        }

        const VkResult result = glfwCreateWindowSurface(getRenderEngine<RenderEngine_Vulkan>()->getVulkanInstance(), windowData->windowGLFW, nullptr, &windowData->vulkanSurface);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create surface for window {}"), windowID);
            GLFW_destroyWindow(windowData);
            m_Windows.remove(windowID);
            return nullptr;
        }

        if (!createWindowSwapchain(windowID, windowData))
        {
            clearWindowDataGLFW(windowID, *windowData);
            m_Windows.remove(windowID);
            return nullptr;
        }
        return windowData;
    }

    void WindowController_Vulkan_GLFW::destroyWindow(const window_id windowID)
    {
        WindowData_Vulkan_GLFW* windowData = m_Windows.find(windowID);
        if (windowData != nullptr)
        {
            clearWindowDataGLFW(windowID, *windowData);
            m_Windows.remove(windowID);
        }
    }
    void WindowController_Vulkan_GLFW::clearWindowDataGLFW(const window_id windowID, WindowData_Vulkan_GLFW& windowData)
    {
        clearWindowDataVulkan(windowID, windowData);
        GLFW_destroyWindow(&windowData);
    }

    bool WindowController_Vulkan_GLFW::shouldCloseWindow(const window_id windowID) const
    {
        const WindowData_Vulkan_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Can't find window {}"), windowID);
            return false;
        }
        return GLFW_shouldCloseWindow(windowData);
    }

    void WindowController_Vulkan_GLFW::updateWindows()
    {
        GLFW_pushWindowEvents();
        Super::updateWindows();
    }
}

#endif
