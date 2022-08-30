// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_LIB_GLFW)

#include "../../include/JumaRE/window/WindowController.h"

#include <GLFW/glfw3.h>

#include "../../include/JumaRE/RenderAPI.h"

namespace JumaRenderEngine
{
    template<typename BaseWindowData, TEMPLATE_ENABLE(is_base_and_not_same<WindowData, BaseWindowData>)>
    struct WindowData_GLFW : BaseWindowData
    {
        GLFWwindow* windowGLFW = nullptr;

        math::ivector2 originalWindowPosition;
        math::ivector2 originalMonitorSize;
        int32 originalMonitorRedreshRate = 0;

        math::ivector2 cachedCursorPosotion = { 0, 0 };
    };

    template<typename BaseWindowController, TEMPLATE_ENABLE(is_base_and_not_same<WindowController, BaseWindowController>)>
    class WindowController_GLFW : public BaseWindowController
    {
        using Super = BaseWindowController;

    public:
        WindowController_GLFW() = default;
        virtual ~WindowController_GLFW() override { clearData_GLFW(); }

        using WindowDataType = WindowData_GLFW<typename Super::WindowDataType>;

        virtual bool shouldCloseWindow(window_id windowID) const override;

        virtual void updateWindows() override;

    protected:

        virtual bool initWindowController() override;

        bool createWindowGLFW(window_id windowID, WindowDataType* windowData, const math::uvector2& size, const jstring& title, 
            GLFWwindow* sharedWindow = nullptr);
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) override;

        virtual bool setMainWindowModeInternal(WindowMode windowMode) override;

        virtual bool setCursorLockedToMainWindowInternal(bool locked) override;

    private:

        static inline WindowController_GLFW* GlobalWindowController = nullptr;


        void clearData_GLFW();

        static void GLFW_ErrorCallback(const int errorCode, const char* errorMessage) { JUTILS_LOG(error, JSTR("GLFW error. Code: {:#x}. {}"), errorCode, errorMessage); }
        static void GLFW_MonitorCallback(GLFWmonitor* monitor, int eventCode);
        void onMonitorDisconnected(GLFWmonitor* monitor);

        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);
        static void GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, int minimized);
        static void GLFW_WindowFocusCallback(GLFWwindow* window, int focused);
        static void GLFW_JoystickCallback(int joystickID, int eventCode);

        static void GLFW_KeyboarbCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void GLFW_MouseButtonCallback(GLFWwindow* window, int buttonCode, int action, int mods);
        static void GLFW_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        static void GLFW_CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
        static void GLFW_TextInputCallback(GLFWwindow* window, unsigned int codepoint);

        void gatherGamepadsInput();
    };

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::initWindowController()
    {
        if (!Super::initWindowController())
        {
            return false;
        }

        if (glfwInit() == GLFW_FALSE)
        {
#ifndef JUTILS_LOG_DISABLED
            const char* errorStr = nullptr;
            glfwGetError(&errorStr);
            JUTILS_LOG(error, JSTR("Failed to initialize GLFW lib: {}"), errorStr);
#endif
            return false;
        }

        GlobalWindowController = this;
        glfwSetErrorCallback(WindowController_GLFW::GLFW_ErrorCallback);
        glfwSetMonitorCallback(WindowController_GLFW::GLFW_MonitorCallback);
        if (Super::API == RenderAPI::OpenGL)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_SAMPLES, 0);
        }
        else
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        return true;
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::clearData_GLFW()
    {
        GlobalWindowController = nullptr;
        glfwTerminate();
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_MonitorCallback(GLFWmonitor* monitor, const int eventCode)
    {
        if (eventCode == GLFW_DISCONNECTED)
        {
            GlobalWindowController->onMonitorDisconnected(monitor);
        }
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::onMonitorDisconnected(GLFWmonitor* monitor)
    {
        const WindowData* windowData = static_cast<WindowData*>(glfwGetMonitorUserPointer(monitor));
        if ((windowData != nullptr) && (windowData->windowID == this->getMainWindowID()))
        {
            this->updateMainWindowMode(WindowMode::Normal);
        }
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::createWindowGLFW(const window_id windowID, WindowDataType* windowData, 
        const math::uvector2& size, const jstring& title, GLFWwindow* sharedWindow)
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        GLFWwindow* window = glfwCreateWindow(
            static_cast<int>(size.x), static_cast<int>(size.y), *title, nullptr, sharedWindow
        );
        if (window == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create GLFW window"));
            return false;
        }

        windowData->windowGLFW = window;
        glfwSetWindowUserPointer(window, windowData);

        double x = 0, y = 0;
        glfwGetCursorPos(window, &x, &y);
        windowData->cachedCursorPosotion = { math::roundDown(static_cast<float>(x)), math::roundDown(static_cast<float>(y)) };

        for (gamepad_index_type gamepadIndex = GLFW_JOYSTICK_1; gamepadIndex <= GLFW_JOYSTICK_16; gamepadIndex++)
        {
            if (glfwJoystickIsGamepad(gamepadIndex) == GLFW_TRUE)
            {
                GlobalWindowController->updateGamepadConnected(gamepadIndex, true);
            }
        }

        glfwSetFramebufferSizeCallback(window, WindowController_GLFW::GLFW_FramebufferResizeCallback);
        glfwSetWindowIconifyCallback(window, WindowController_GLFW::GLFW_WindowMinimizationCallback);
        glfwSetWindowFocusCallback(window, WindowController_GLFW::GLFW_WindowFocusCallback);
        glfwSetJoystickCallback(WindowController_GLFW::GLFW_JoystickCallback);

        glfwSetKeyCallback(window, WindowController_GLFW::GLFW_KeyboarbCallback);
        glfwSetMouseButtonCallback(window, WindowController_GLFW::GLFW_MouseButtonCallback);
        glfwSetScrollCallback(window, WindowController_GLFW::GLFW_ScrollCallback);
        glfwSetCursorPosCallback(window, WindowController_GLFW::GLFW_CursorPositionCallback);
        glfwSetCharCallback(window, WindowController_GLFW::GLFW_TextInputCallback);
        return true;
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::destroyWindowInternal(const window_id windowID, WindowData* windowData)
    {
        Super::destroyWindowInternal(windowID, windowData);

        WindowDataType* windowDataGLFW = reinterpret_cast<WindowDataType*>(windowData);
        if (windowDataGLFW->windowGLFW != nullptr)
        {
            glfwDestroyWindow(windowDataGLFW->windowGLFW);
            windowDataGLFW->windowGLFW = nullptr;
        }
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, const int width, 
        const int height)
    {
        const WindowData* windowData = static_cast<WindowData*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            GlobalWindowController->updateWindowSize(windowData->windowID, math::uvector2(static_cast<uint32>(width), static_cast<uint32>(height)));
        }
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, const int minimized)
    {
        const WindowData* windowData = static_cast<WindowData*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            GlobalWindowController->updateWindowMinimization(windowData->windowID, minimized == GLFW_TRUE);
        }
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_WindowFocusCallback(GLFWwindow* window, const int focused)
    {
        const WindowData* windowData = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        if (windowData != nullptr)
        {
            GlobalWindowController->updateWindowFocused(windowData->windowID, focused == GLFW_TRUE);
        }
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_JoystickCallback(const int joystickID, const int eventCode)
    {
        if (eventCode == GLFW_CONNECTED)
        {
            if (glfwJoystickIsGamepad(joystickID) == GLFW_TRUE)
            {
                GlobalWindowController->updateGamepadConnected(static_cast<gamepad_index_type>(joystickID), true);
            }
        }
        else if (eventCode == GLFW_DISCONNECTED)
        {
            GlobalWindowController->updateGamepadConnected(static_cast<gamepad_index_type>(joystickID), false);
        }
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::shouldCloseWindow(const window_id windowID) const
    {
        const WindowDataType* windowData = this->template findWindowData<WindowDataType>(windowID);
        return (windowData != nullptr) && (glfwWindowShouldClose(windowData->windowGLFW) == GLFW_TRUE);
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::setMainWindowModeInternal(const WindowMode windowMode)
    {
        WindowDataType* windowData = this->template getWindowData<WindowDataType>(this->getMainWindowID());
        if (windowData == nullptr)
        {
            return false;
        }
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (monitor == nullptr)
        {
            return false;
        }

        if (this->getMainWindowMode() == WindowMode::Normal)
        {
            const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
            if (videoMode == nullptr)
            {
                return false;
            }

            glfwGetWindowPos(windowData->windowGLFW, &windowData->originalWindowPosition.x, &windowData->originalWindowPosition.y);
            windowData->originalMonitorSize = { videoMode->width, videoMode->height };
            windowData->originalMonitorRedreshRate = videoMode->refreshRate;

            glfwSetMonitorUserPointer(monitor, windowData);
        }

        switch (windowMode)
        {
        case WindowMode::Normal: 
            glfwSetWindowMonitor(windowData->windowGLFW, nullptr, 
                windowData->originalWindowPosition.x, windowData->originalWindowPosition.y, 
                windowData->desiredSize.x, windowData->desiredSize.y, GLFW_DONT_CARE
            );
            glfwSetMonitorUserPointer(monitor, nullptr);
            break;
        case WindowMode::Fullscreen: 
            glfwSetWindowMonitor(windowData->windowGLFW, monitor, 0, 0, 
                windowData->desiredSize.x, windowData->desiredSize.y, GLFW_DONT_CARE
            );
            break;
        case WindowMode::WindowedFullscreen: 
            glfwSetWindowMonitor(windowData->windowGLFW, monitor, 0, 0, 
                windowData->originalMonitorSize.x, windowData->originalMonitorSize.y, windowData->originalMonitorRedreshRate
            );
            break;
        default: ;
        }
        return true;
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::setCursorLockedToMainWindowInternal(const bool locked)
    {
        WindowDataType* windowData = this->template getWindowData<WindowDataType>(this->getMainWindowID());
        if (windowData == nullptr)
        {
            return false;
        }
        glfwSetInputMode(windowData->windowGLFW, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        return true;
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::updateWindows()
    {
        glfwPollEvents();

        gatherGamepadsInput();

        Super::updateWindows();
    }

    constexpr InputButtonAction GetInputButtonActionByGLFW(const int action)
    {
        switch (action)
        {
        case GLFW_PRESS:   return InputButtonAction::Press;
        case GLFW_REPEAT:  return InputButtonAction::Repeate;
        case GLFW_RELEASE: return InputButtonAction::Release;
        default: ;
        }
        return InputButtonAction::Release;
    }
    constexpr input_mods_type GetInputModsByGLFW(const int mods)
    {
        input_mods_type inputMods = 0;
        if (mods & GLFW_MOD_SHIFT)
        {
            inputMods |= INPUT_MOD_FLAG_SHIFT;
        }
        if (mods & GLFW_MOD_CONTROL)
        {
            inputMods |= INPUT_MOD_FLAG_CONTROL;
        }
        if (mods & GLFW_MOD_ALT)
        {
            inputMods |= INPUT_MOD_FLAG_ALT;
        }
        if (mods & GLFW_MOD_SUPER)
        {
            inputMods |= INPUT_MOD_FLAG_SUPER;
        }
        if (mods & GLFW_MOD_CAPS_LOCK)
        {
            inputMods |= INPUT_MOD_FLAG_CAPS_LOCK;
        }
        if (mods & GLFW_MOD_NUM_LOCK)
        {
            inputMods |= INPUT_MOD_FLAG_NUM_LOCK;
        }
        return inputMods;
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_KeyboarbCallback(GLFWwindow* window, const int key, const int scancode, 
        const int action, const int mods)
    {
        const WindowDataType* windowData = static_cast<WindowDataType*>(glfwGetWindowUserPointer(window));
        if (windowData == nullptr)
        {
            return;
        }

        InputButton button = InputButton::NONE;
        switch (key)
        {
        case GLFW_KEY_SPACE: button = InputButton::Space; break;
        case GLFW_KEY_APOSTROPHE: button = InputButton::Apostrophe; break;
        case GLFW_KEY_COMMA: button = InputButton::Comma; break;
        case GLFW_KEY_MINUS: button = InputButton::Minus; break;
        case GLFW_KEY_PERIOD: button = InputButton::Dot; break;
        case GLFW_KEY_SLASH: button = InputButton::Slash; break;
        case GLFW_KEY_0: button = InputButton::Num0; break;
        case GLFW_KEY_1: button = InputButton::Num1; break;
        case GLFW_KEY_2: button = InputButton::Num2; break;
        case GLFW_KEY_3: button = InputButton::Num3; break;
        case GLFW_KEY_4: button = InputButton::Num4; break;
        case GLFW_KEY_5: button = InputButton::Num5; break;
        case GLFW_KEY_6: button = InputButton::Num6; break;
        case GLFW_KEY_7: button = InputButton::Num7; break;
        case GLFW_KEY_8: button = InputButton::Num8; break;
        case GLFW_KEY_9: button = InputButton::Num9; break;
        case GLFW_KEY_SEMICOLON: button = InputButton::Semicolon; break;
        case GLFW_KEY_EQUAL: button = InputButton::Equal; break;
        case GLFW_KEY_A: button = InputButton::A; break;
        case GLFW_KEY_B: button = InputButton::B; break;
        case GLFW_KEY_C: button = InputButton::C; break;
        case GLFW_KEY_D: button = InputButton::D; break;
        case GLFW_KEY_E: button = InputButton::E; break;
        case GLFW_KEY_F: button = InputButton::F; break;
        case GLFW_KEY_G: button = InputButton::G; break;
        case GLFW_KEY_H: button = InputButton::H; break;
        case GLFW_KEY_I: button = InputButton::I; break;
        case GLFW_KEY_J: button = InputButton::J; break;
        case GLFW_KEY_K: button = InputButton::K; break;
        case GLFW_KEY_L: button = InputButton::L; break;
        case GLFW_KEY_M: button = InputButton::M; break;
        case GLFW_KEY_N: button = InputButton::N; break;
        case GLFW_KEY_O: button = InputButton::O; break;
        case GLFW_KEY_P: button = InputButton::P; break;
        case GLFW_KEY_Q: button = InputButton::Q; break;
        case GLFW_KEY_R: button = InputButton::R; break;
        case GLFW_KEY_S: button = InputButton::S; break;
        case GLFW_KEY_T: button = InputButton::T; break;
        case GLFW_KEY_U: button = InputButton::U; break;
        case GLFW_KEY_V: button = InputButton::V; break;
        case GLFW_KEY_W: button = InputButton::W; break;
        case GLFW_KEY_X: button = InputButton::X; break;
        case GLFW_KEY_Y: button = InputButton::Y; break;
        case GLFW_KEY_Z: button = InputButton::Z; break;
        case GLFW_KEY_LEFT_BRACKET: button = InputButton::BracketLeft; break;
        case GLFW_KEY_BACKSLASH: button = InputButton::Backslash; break;
        case GLFW_KEY_RIGHT_BRACKET: button = InputButton::BracketRight; break;
        case GLFW_KEY_GRAVE_ACCENT: button = InputButton::GraveAccent; break;
        case GLFW_KEY_ESCAPE: button = InputButton::Escape; break;
        case GLFW_KEY_ENTER: button = InputButton::Enter; break;
        case GLFW_KEY_TAB: button = InputButton::Tab; break;
        case GLFW_KEY_BACKSPACE: button = InputButton::Backspace; break;
        case GLFW_KEY_INSERT: button = InputButton::Insert; break;
        case GLFW_KEY_DELETE: button = InputButton::Delete; break;
        case GLFW_KEY_RIGHT: button = InputButton::ArrowRight; break;
        case GLFW_KEY_LEFT: button = InputButton::ArrowLeft; break;
        case GLFW_KEY_DOWN: button = InputButton::ArrowDown; break;
        case GLFW_KEY_UP: button = InputButton::ArrowUp; break;
        case GLFW_KEY_PAGE_UP: button = InputButton::PageUp; break;
        case GLFW_KEY_PAGE_DOWN: button = InputButton::PageDown; break;
        case GLFW_KEY_HOME: button = InputButton::Home; break;
        case GLFW_KEY_END: button = InputButton::End; break;
        case GLFW_KEY_CAPS_LOCK: button = InputButton::CapsLock; break;
        case GLFW_KEY_SCROLL_LOCK: button = InputButton::ScrollLock; break;
        case GLFW_KEY_NUM_LOCK: button = InputButton::NumLock; break;
        case GLFW_KEY_PRINT_SCREEN: button = InputButton::PrintScreen; break;
        case GLFW_KEY_PAUSE: button = InputButton::Pause; break;
        case GLFW_KEY_F1: button = InputButton::F1; break;
        case GLFW_KEY_F2: button = InputButton::F2; break;
        case GLFW_KEY_F3: button = InputButton::F3; break;
        case GLFW_KEY_F4: button = InputButton::F4; break;
        case GLFW_KEY_F5: button = InputButton::F5; break;
        case GLFW_KEY_F6: button = InputButton::F6; break;
        case GLFW_KEY_F7: button = InputButton::F7; break;
        case GLFW_KEY_F8: button = InputButton::F8; break;
        case GLFW_KEY_F9: button = InputButton::F9; break;
        case GLFW_KEY_F10: button = InputButton::F10; break;
        case GLFW_KEY_F11: button = InputButton::F11; break;
        case GLFW_KEY_F12: button = InputButton::F12; break;
        case GLFW_KEY_KP_0: button = InputButton::NumPad0; break;
        case GLFW_KEY_KP_1: button = InputButton::NumPad1; break;
        case GLFW_KEY_KP_2: button = InputButton::NumPad2; break;
        case GLFW_KEY_KP_3: button = InputButton::NumPad3; break;
        case GLFW_KEY_KP_4: button = InputButton::NumPad4; break;
        case GLFW_KEY_KP_5: button = InputButton::NumPad5; break;
        case GLFW_KEY_KP_6: button = InputButton::NumPad6; break;
        case GLFW_KEY_KP_7: button = InputButton::NumPad7; break;
        case GLFW_KEY_KP_8: button = InputButton::NumPad8; break;
        case GLFW_KEY_KP_9: button = InputButton::NumPad9; break;
        case GLFW_KEY_KP_DECIMAL: button = InputButton::NumPadDot; break;
        case GLFW_KEY_KP_DIVIDE: button = InputButton::NumPadDivide; break;
        case GLFW_KEY_KP_MULTIPLY: button = InputButton::NumPadMultiply; break;
        case GLFW_KEY_KP_SUBTRACT: button = InputButton::NumPadMinus; break;
        case GLFW_KEY_KP_ADD: button = InputButton::NumPadPlus; break;
        case GLFW_KEY_KP_ENTER: button = InputButton::NumPadEnter; break;
        case GLFW_KEY_LEFT_SHIFT: button = InputButton::ShiftLeft; break;
        case GLFW_KEY_LEFT_CONTROL: button = InputButton::ControlLeft; break;
        case GLFW_KEY_LEFT_ALT: button = InputButton::AltLeft; break;
        case GLFW_KEY_LEFT_SUPER: button = InputButton::SuperLeft; break;
        case GLFW_KEY_RIGHT_SHIFT: button = InputButton::ShiftRight; break;
        case GLFW_KEY_RIGHT_CONTROL: button = InputButton::ControlRight; break;
        case GLFW_KEY_RIGHT_ALT: button = InputButton::AltRight; break;
        case GLFW_KEY_RIGHT_SUPER: button = InputButton::SuperRight; break;
        default: return;
        }
        const InputButtonAction buttonAction = GetInputButtonActionByGLFW(action);
        const input_mods_type inputMods = GetInputModsByGLFW(mods);

        GlobalWindowController->updateWindowInputButtonState(windowData->windowID, InputDeviceType::Keyboard, button, buttonAction, inputMods);
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_MouseButtonCallback(GLFWwindow* window, const int buttonCode, 
        const int action, const int mods)
    {
        const WindowDataType* windowData = static_cast<WindowDataType*>(glfwGetWindowUserPointer(window));
        if (windowData == nullptr)
        {
            return;
        }

        InputButton button = InputButton::NONE;
        switch (buttonCode)
        {
        case GLFW_MOUSE_BUTTON_1: button = InputButton::MouseLeft; break;
        case GLFW_MOUSE_BUTTON_2: button = InputButton::MouseRight; break;
        case GLFW_MOUSE_BUTTON_3: button = InputButton::MouseWheel; break;
        case GLFW_MOUSE_BUTTON_4: button = InputButton::MouseButton4; break;
        case GLFW_MOUSE_BUTTON_5: button = InputButton::MouseButton5; break;
        case GLFW_MOUSE_BUTTON_6: button = InputButton::MouseButton6; break;
        case GLFW_MOUSE_BUTTON_7: button = InputButton::MouseButton7; break;
        case GLFW_MOUSE_BUTTON_8: button = InputButton::MouseButton8; break;
        default: return;
        }
        const InputButtonAction buttonAction = GetInputButtonActionByGLFW(action);
        const input_mods_type inputMods = GetInputModsByGLFW(mods);

        GlobalWindowController->updateWindowInputButtonState(windowData->windowID, InputDeviceType::Mouse, button, buttonAction, inputMods);
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_ScrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
    {
        const WindowDataType* windowData = static_cast<WindowDataType*>(glfwGetWindowUserPointer(window));
        if (windowData == nullptr)
        {
            return;
        }

        constexpr InputDeviceType device = InputDeviceType::Mouse;
        constexpr InputAxis axis = InputAxis::MouseWheel;
        const float offset = static_cast<float>(yoffset);

        GlobalWindowController->updateWindowInputAxisState(windowData->windowID, InputDeviceType::Mouse, InputAxis::MouseWheel, { offset, 0.0f }, 0);
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_CursorPositionCallback(GLFWwindow* window, const double xpos, 
        const double ypos)
    {
        WindowDataType* windowData = static_cast<WindowDataType*>(glfwGetWindowUserPointer(window));
        if (windowData == nullptr)
        {
            return;
        }

        const math::ivector2 position = { math::roundDown(static_cast<float>(xpos)), math::roundDown(static_cast<float>(ypos)) };
        const math::ivector2 offset = position - windowData->cachedCursorPosotion;
        windowData->cachedCursorPosotion = position;
        GlobalWindowController->updateWindowCursorPosition(windowData->windowID, position, offset);
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_TextInputCallback(GLFWwindow* window, const unsigned int codepoint)
    {
        const WindowDataType* windowData = static_cast<WindowDataType*>(glfwGetWindowUserPointer(window));
        if (windowData == nullptr)
        {
            return;
        }

        constexpr uint32 CODE_POINT_MAX = 0x0010FFFFu;
        constexpr uint16 LEAD_SURROGATE_MIN = 0xD800u;
        constexpr uint16 TRAIL_SURROGATE_MAX = 0xDFFFu;
        if ((codepoint > CODE_POINT_MAX) || ((codepoint >= LEAD_SURROGATE_MIN) && (codepoint <= TRAIL_SURROGATE_MAX)))
        {
            return;
        }

        char str[5] = { 0, 0, 0, 0, 0 };
        if (codepoint < 0x80)
        {
            str[0] = static_cast<char>(codepoint);
        }
        else if (codepoint < 0x800)
        {
            str[0] = static_cast<char>((codepoint >> 6) | 0xC0);
            str[1] = static_cast<char>((codepoint & 0x3F) | 0x80);
        }
        else if (codepoint < 0x10000)
        {
            str[0] = static_cast<char>((codepoint >> 12) | 0xE0);
            str[1] = static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
            str[2] = static_cast<char>((codepoint & 0x3F) | 0x80);
        }
        else
        {
            str[0] = static_cast<char>((codepoint >> 18) | 0xF0);
            str[1] = static_cast<char>(((codepoint >> 12) & 0x3F) | 0x80);
            str[2] = static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
            str[3] = static_cast<char>((codepoint & 0x3F) | 0x80);
        }
        // TODO: Text input
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::gatherGamepadsInput()
    {
        const window_id focusedWindowID = GlobalWindowController->getFocusedWindowID();
        for (const auto& gamepadIndex : GlobalWindowController->getConnectedGamepads())
        {
            GLFWgamepadstate state;
            if (glfwGetGamepadState(gamepadIndex, &state) == GLFW_FALSE)
            {
                continue;
            }
            const InputDeviceType device = GetGamepadDeviceByIndex(gamepadIndex);

            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadA, 
                state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadB, 
                state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadX, 
                state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadY, 
                state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadBumperLeft, 
                state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadBumperRight, 
                state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadTriggerLeft, 
                state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadTriggerRight, 
                state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadBack, 
                state.buttons[GLFW_GAMEPAD_BUTTON_BACK] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadStart, 
                state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadGuide, 
                state.buttons[GLFW_GAMEPAD_BUTTON_GUIDE] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadDPadUp, 
                state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadDPadRight, 
                state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadDPadDown, 
                state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);
            GlobalWindowController->updateWindowInputButtonState(focusedWindowID, device, InputButton::GamepadDPadLeft, 
                state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS ? InputButtonAction::Press : InputButtonAction::Release);

            GlobalWindowController->updateWindowInputAxisState(focusedWindowID, device, InputAxis::GamepadTriggerLeft, 
                { state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER], 0.0f });
            GlobalWindowController->updateWindowInputAxisState(focusedWindowID, device, InputAxis::GamepadTriggerRight, 
                { state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER], 0.0f });
            GlobalWindowController->updateWindowInputAxisState(focusedWindowID, device, InputAxis::GamepadStick2DLeft, 
                { state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] });
            GlobalWindowController->updateWindowInputAxisState(focusedWindowID, device, InputAxis::GamepadStick2DRight, 
                { state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X], state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] });
        }
    }
}

#endif
