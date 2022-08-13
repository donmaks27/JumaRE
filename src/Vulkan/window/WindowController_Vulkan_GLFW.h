// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_Vulkan.h"
#include "../../GLFW/WindowController_GLFW.h"

namespace JumaRenderEngine
{
    struct WindowData_Vulkan_GLFW : WindowData_Vulkan, WindowData_GLFW
    {
    };

    class WindowController_Vulkan_GLFW final : public WindowController_Vulkan, protected WindowController_GLFW
    {
        using Super = WindowController_Vulkan;

    public:
        WindowController_Vulkan_GLFW() = default;
        virtual ~WindowController_Vulkan_GLFW() override;

        virtual jarray<const char*> getVulkanInstanceExtensions() const override;

        virtual void destroyWindow(window_id windowID) override;

        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }
        virtual jarray<window_id> getWindowIDs() const override { return m_Windows.getKeys(); }

        virtual bool shouldCloseWindow(window_id windowID) const override;

        virtual void updateWindows() override;

    protected:

        virtual bool initWindowController() override;

        virtual WindowData* createWindowInternal(window_id windowID, const WindowInitProperties& properties) override;

        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }
        
        virtual void GLFW_onWindowResized(WindowData* windowData, const math::uvector2& size) override { updateWindowSize(windowData->windowID, size); }
        virtual void GLFW_onWindowMinimizationChanged(WindowData* windowData, const bool minimized) override { updateWindowMinimization(windowData->windowID, minimized); }

        virtual void setWindowTitleInternal(WindowData* windowData, const jstring& title) override { GLFW_setWindowTitle(reinterpret_cast<const WindowData_Vulkan_GLFW*>(windowData), title); }

    private:

        jmap<window_id, WindowData_Vulkan_GLFW> m_Windows;


        void clearGLFW();

        void clearWindowDataGLFW(window_id windowID, WindowData_Vulkan_GLFW& windowData);
    };
}

#endif
