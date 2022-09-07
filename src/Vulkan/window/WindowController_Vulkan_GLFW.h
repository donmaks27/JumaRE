// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN) && defined(GLFW_ENABLED)

#include "WindowController_Vulkan.h"
#include "../../GLFW/WindowController_GLFW.h"

namespace JumaRenderEngine
{
    using WindowData_Vulkan_GLFW = WindowData_GLFW<WindowData_Vulkan>;

    class WindowController_Vulkan_GLFW final : public WindowController_GLFW<WindowController_Vulkan>
    {
        using Super = WindowController_GLFW<WindowController_Vulkan>;

    public:
        WindowController_Vulkan_GLFW() = default;
        virtual ~WindowController_Vulkan_GLFW() override;

        using WindowDataType = WindowData_Vulkan_GLFW;


        virtual jarray<const char*> getVulkanInstanceExtensions() const override;

        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }

    protected:

        virtual WindowData* createWindowInternal(window_id windowID, const WindowCreateInfo& createInfo) override;
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) override;

        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }

    private:

        jmap<window_id, WindowDataType> m_Windows;


        void clearData_Vulkan_GLFW();
    };
}

#endif
