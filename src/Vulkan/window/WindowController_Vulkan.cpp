﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "WindowController_Vulkan.h"

#include "../RenderEngine_Vulkan.h"
#include "../vulkanObjects/VulkanSwapchain.h"

namespace JumaRenderEngine
{
    WindowController_Vulkan::~WindowController_Vulkan()
    {
        clearData_Vulkan();
    }

    void WindowController_Vulkan::clearData_Vulkan()
    {
    }

    void WindowController_Vulkan::clearWindowDataInternal(WindowData* windowData)
    {
        Super::clearWindowDataInternal(windowData);

        WindowData_Vulkan* windowDataVulkan = reinterpret_cast<WindowData_Vulkan*>(windowData);
        destroyWindowSwapchain(windowDataVulkan);

        vkDestroySurfaceKHR(getRenderEngine<RenderEngine_Vulkan>()->getVulkanInstance(), windowDataVulkan->vulkanSurface, nullptr);
        windowDataVulkan->vulkanSurface = nullptr;
    }

    bool WindowController_Vulkan::createWindowSwapchain(const window_id windowID, WindowData_Vulkan* windowData)
    {
        if (windowData->vulkanSwapchain == nullptr)
        {
            RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
            if (renderEngine->getDevice() == nullptr)
            {
                // Vulkan device is not created yet
                return true;
            }

            VulkanSwapchain* swapchain = renderEngine->createObject<VulkanSwapchain>();
            if (!swapchain->init(windowID))
            {
                JUTILS_LOG(error, JSTR("Failed to create vulkan swapchain for window {}"), windowID);
                delete swapchain;
                return false;
            }
            windowData->vulkanSwapchain = swapchain;
        }
        return true;
    }
    void WindowController_Vulkan::destroyWindowSwapchain(WindowData_Vulkan* windowData)
    {
        if (windowData->vulkanSwapchain != nullptr)
        {
            delete windowData->vulkanSwapchain;
            windowData->vulkanSwapchain = nullptr;
        }
    }

    bool WindowController_Vulkan::createWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createWindowSwapchain(windowID, getWindowData<WindowData_Vulkan>(windowID)))
            {
                JUTILS_LOG(error, JSTR("Failed to create vulkan swapchain"));
                return false;
            }
        }
        return true;
    }
    void WindowController_Vulkan::destroyWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            destroyWindowSwapchain(getWindowData<WindowData_Vulkan>(windowID));
        }
    }

    void WindowController_Vulkan::onWindowMinimizationChanged(WindowData* windowData)
    {
        Super::onWindowMinimizationChanged(windowData);

        if (windowData->minimized)
        {
            const WindowData_Vulkan* windowDataVulkan = reinterpret_cast<const WindowData_Vulkan*>(windowData);
            if (windowDataVulkan->vulkanSwapchain != nullptr)
            {
                windowDataVulkan->vulkanSwapchain->invalidate();
            }
        }
    }
}

#endif
