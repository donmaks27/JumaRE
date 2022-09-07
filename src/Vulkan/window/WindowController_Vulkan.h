// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../../include/JumaRE/window/WindowController.h"

#include <vulkan/vulkan_core.h>

#include "../../../include/JumaRE/RenderAPI.h"

namespace JumaRenderEngine
{
    class RenderEngine_Vulkan;
    class VulkanSwapchain;

    struct WindowData_Vulkan : WindowData
    {
        VkSurfaceKHR vulkanSurface = nullptr;
        VulkanSwapchain* vulkanSwapchain = nullptr;
    };

    class WindowController_Vulkan : public WindowController
    {
        using Super = WindowController;

        friend RenderEngine_Vulkan;

    public:
        WindowController_Vulkan() = default;
        virtual ~WindowController_Vulkan() override;

        using WindowDataType = WindowData_Vulkan;


        virtual jarray<const char*> getVulkanInstanceExtensions() const = 0;

    protected:

        static constexpr RenderAPI API = RenderAPI::Vulkan;


        virtual void clearWindowDataInternal(window_id windowID, WindowData* windowData) override;

        bool createWindowSwapchain(window_id windowID, WindowData_Vulkan* windowData);

        virtual void onWindowMinimizationChanged(window_id windowID, WindowData* windowData) override;

    private:

        void clearData_Vulkan();

        void destroyWindowSwapchain(window_id windowID, WindowData_Vulkan* windowData);
        bool createWindowSwapchains();
        void destroyWindowSwapchains();
    };
}

#endif
