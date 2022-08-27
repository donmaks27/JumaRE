// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

namespace JumaRenderEngine
{
    enum class InputDeviceType : uint8
    {
        NONE,
        Keyboard,
        Mouse,
        Gamepad1,
        Gamepad2,
        Gamepad3,
        Gamepad4,
        Gamepad5,
        Gamepad6,
        Gamepad7,
        Gamepad8,
        Gamepad9,
        Gamepad10,
        Gamepad11,
        Gamepad12,
        Gamepad13,
        Gamepad14,
        Gamepad15,
        Gamepad16
    };

    using input_button_type = uint8;
    enum class InputButton : input_button_type
    {
        NONE,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Comma,          /* , */
        Dot,            /* . */
        Slash,          /* / */
        Semicolon,      /* ; */
        Apostrophe,     /* ' */
        BracketLeft,    /* [ */
        BracketRight,   /* ] */
        GraveAccent,    /* ` */
        Minus,          /* - */
        Equal,          /* = */
        Backslash,      /* \ */
        Backspace,
        Enter,
        Tab,
        CapsLock,
        ShiftLeft,
        ShiftRight,
        ControlLeft,
        ControlRight,
        AltLeft,
        AltRight,
        SuperLeft,
        SuperRight,
        Space,
        Escape,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        ArrowRight,
        ArrowLeft,
        ArrowDown,
        ArrowUp,
        Insert,
        Delete,
        Home,
        End,
        PageUp,
        PageDown,
        PrintScreen,
        ScrollLock,
        Pause,
        NumLock,
        NumPad0,
        NumPad1,
        NumPad2,
        NumPad3,
        NumPad4,
        NumPad5,
        NumPad6,
        NumPad7,
        NumPad8,
        NumPad9,
        NumPadDot,
        NumPadDivide,
        NumPadMultiply,
        NumPadMinus,
        NumPadPlus,
        NumPadEnter,
        MouseLeft,
        MouseRight,
        MouseWheel,
        MouseButton4,
        MouseButton5,
        MouseButton6,
        MouseButton7,
        MouseButton8,
        GamepadA,
        GamepadB,
        GamepadX,
        GamepadY,
        GamepadBumperLeft,
        GamepadBumperRight,
        GamepadTriggerLeft,
        GamepadTriggerRight,
        GamepadBack,
        GamepadStart,
        GamepadGuide,
        GamepadDPadUp,
        GamepadDPadRight,
        GamepadDPadDown,
        GamepadDPadLeft
    };
    enum class InputButtonAction : uint8
    {
        Press,
        Repeate,
        Release
    };

    enum class InputAxis
    {
        NONE,
        Mouse2D,
        MouseWheel,
        GamepadTriggerLeft,
        GamepadTriggerRight,
        GamepadStick2DLeft,
        GamepadStick2DRight
    };

    using input_mods_type = uint8;
    enum InputModFags : input_mods_type
    {
        INPUT_MOD_FLAG_SHIFT = 1,
        INPUT_MOD_FLAG_CONTROL = 2,
        INPUT_MOD_FLAG_ALT = 4,
        INPUT_MOD_FLAG_SUPER = 8,
        INPUT_MOD_FLAG_CAPS_LOCK = 16,
        INPUT_MOD_FLAG_NUM_LOCK = 32
    };

    constexpr bool IsInputDeviceContainsInputButton(const InputDeviceType device, const InputButton button)
    {
        if ((device == InputDeviceType::NONE) || (button == InputButton::NONE))
        {
            return false;
        }
        switch (button)
        {
        case InputButton::MouseLeft:
        case InputButton::MouseRight:
        case InputButton::MouseWheel:
        case InputButton::MouseButton4:
        case InputButton::MouseButton5:
        case InputButton::MouseButton6:
        case InputButton::MouseButton7:
        case InputButton::MouseButton8:
            return device == InputDeviceType::Mouse;

        case InputButton::GamepadA:
        case InputButton::GamepadB:
        case InputButton::GamepadX:
        case InputButton::GamepadY:
        case InputButton::GamepadBumperLeft:
        case InputButton::GamepadBumperRight:
        case InputButton::GamepadTriggerLeft:
        case InputButton::GamepadTriggerRight:
        case InputButton::GamepadBack:
        case InputButton::GamepadStart:
        case InputButton::GamepadGuide:
        case InputButton::GamepadDPadUp:
        case InputButton::GamepadDPadRight:
        case InputButton::GamepadDPadDown:
        case InputButton::GamepadDPadLeft:
            switch (device)
            {
            case InputDeviceType::Gamepad1:
            case InputDeviceType::Gamepad2:
            case InputDeviceType::Gamepad3:
            case InputDeviceType::Gamepad4:
            case InputDeviceType::Gamepad5:
            case InputDeviceType::Gamepad6:
            case InputDeviceType::Gamepad7:
            case InputDeviceType::Gamepad8:
            case InputDeviceType::Gamepad9:
            case InputDeviceType::Gamepad10:
            case InputDeviceType::Gamepad11:
            case InputDeviceType::Gamepad12:
            case InputDeviceType::Gamepad13:
            case InputDeviceType::Gamepad14:
            case InputDeviceType::Gamepad15:
            case InputDeviceType::Gamepad16:
                return true;
            default: ;
            }
            return false;

        default: ;
        }
        return device == InputDeviceType::Keyboard;
    }
    constexpr bool IsInputDeviceContainsInputAxis(const InputDeviceType device, const InputAxis axis)
    {
        if ((device == InputDeviceType::NONE) || (axis == InputAxis::NONE))
        {
            return false;
        }
        switch (axis)
        {
        case InputAxis::Mouse2D:
        case InputAxis::MouseWheel:
            return device == InputDeviceType::Mouse;

        case InputAxis::GamepadTriggerLeft:
        case InputAxis::GamepadTriggerRight:
        case InputAxis::GamepadStick2DLeft:
        case InputAxis::GamepadStick2DRight:
            switch (device)
            {
            case InputDeviceType::Gamepad1:
            case InputDeviceType::Gamepad2:
            case InputDeviceType::Gamepad3:
            case InputDeviceType::Gamepad4:
            case InputDeviceType::Gamepad5:
            case InputDeviceType::Gamepad6:
            case InputDeviceType::Gamepad7:
            case InputDeviceType::Gamepad8:
            case InputDeviceType::Gamepad9:
            case InputDeviceType::Gamepad10:
            case InputDeviceType::Gamepad11:
            case InputDeviceType::Gamepad12:
            case InputDeviceType::Gamepad13:
            case InputDeviceType::Gamepad14:
            case InputDeviceType::Gamepad15:
            case InputDeviceType::Gamepad16:
                return true;
            default: ;
            }
            break;

        default: ;
        }
        return false;
    }
    constexpr bool IsInputAxis2D(const InputAxis axis)
    {
        switch (axis)
        {
        case InputAxis::Mouse2D:
        case InputAxis::GamepadStick2DLeft:
        case InputAxis::GamepadStick2DRight:
            return true;
        default: ;
        }
        return false;
    }
}
