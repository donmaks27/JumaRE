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

    constexpr const char* InputDeviceTypeToString(const InputDeviceType device)
    {
        switch (device)
        {
        case InputDeviceType::Keyboard: return JSTR("Keyboard");
        case InputDeviceType::Mouse: return JSTR("Mouse");
        case InputDeviceType::Gamepad1: return JSTR("Gamepad1");
        case InputDeviceType::Gamepad2: return JSTR("Gamepad2");
        case InputDeviceType::Gamepad3: return JSTR("Gamepad3");
        case InputDeviceType::Gamepad4: return JSTR("Gamepad4");
        case InputDeviceType::Gamepad5: return JSTR("Gamepad5");
        case InputDeviceType::Gamepad6: return JSTR("Gamepad6");
        case InputDeviceType::Gamepad7: return JSTR("Gamepad7");
        case InputDeviceType::Gamepad8: return JSTR("Gamepad8");
        case InputDeviceType::Gamepad9: return JSTR("Gamepad9");
        case InputDeviceType::Gamepad10: return JSTR("Gamepad10");
        case InputDeviceType::Gamepad11: return JSTR("Gamepad11");
        case InputDeviceType::Gamepad12: return JSTR("Gamepad12");
        case InputDeviceType::Gamepad13: return JSTR("Gamepad13");
        case InputDeviceType::Gamepad14: return JSTR("Gamepad14");
        case InputDeviceType::Gamepad15: return JSTR("Gamepad15");
        case InputDeviceType::Gamepad16: return JSTR("Gamepad16");
        default: ;
        }
        return JSTR("NONE");
    }
    constexpr const char* InputButtonToString(const InputButton button)
    {
        switch (button)
        {
        case InputButton::A:                    return JSTR("A");
        case InputButton::B:                    return JSTR("B");
        case InputButton::C:                    return JSTR("C");
        case InputButton::D:                    return JSTR("D");
        case InputButton::E:                    return JSTR("E");
        case InputButton::F:                    return JSTR("F");
        case InputButton::G:                    return JSTR("G");
        case InputButton::H:                    return JSTR("H");
        case InputButton::I:                    return JSTR("I");
        case InputButton::J:                    return JSTR("J");
        case InputButton::K:                    return JSTR("K");
        case InputButton::L:                    return JSTR("L");
        case InputButton::M:                    return JSTR("M");
        case InputButton::N:                    return JSTR("N");
        case InputButton::O:                    return JSTR("O");
        case InputButton::P:                    return JSTR("P");
        case InputButton::Q:                    return JSTR("Q");
        case InputButton::R:                    return JSTR("R");
        case InputButton::S:                    return JSTR("S");
        case InputButton::T:                    return JSTR("T");
        case InputButton::U:                    return JSTR("U");
        case InputButton::V:                    return JSTR("V");
        case InputButton::W:                    return JSTR("W");
        case InputButton::X:                    return JSTR("X");
        case InputButton::Y:                    return JSTR("Y");
        case InputButton::Z:                    return JSTR("Z");
        case InputButton::Num0:                 return JSTR("Num0");
        case InputButton::Num1:                 return JSTR("Num1");
        case InputButton::Num2:                 return JSTR("Num2");
        case InputButton::Num3:                 return JSTR("Num3");
        case InputButton::Num4:                 return JSTR("Num4");
        case InputButton::Num5:                 return JSTR("Num5");
        case InputButton::Num6:                 return JSTR("Num6");
        case InputButton::Num7:                 return JSTR("Num7");
        case InputButton::Num8:                 return JSTR("Num8");
        case InputButton::Num9:                 return JSTR("Num9");
        case InputButton::Comma:                return JSTR("Comma");
        case InputButton::Dot:                  return JSTR("Dot");
        case InputButton::Slash:                return JSTR("Slash");
        case InputButton::Semicolon:            return JSTR("Semicolon");
        case InputButton::Apostrophe:           return JSTR("Apostrophe");
        case InputButton::BracketLeft:          return JSTR("BracketLeft");
        case InputButton::BracketRight:         return JSTR("BracketRight");
        case InputButton::GraveAccent:          return JSTR("GraveAccent");
        case InputButton::Minus:                return JSTR("Minus");
        case InputButton::Equal:                return JSTR("Equal");
        case InputButton::Backslash:            return JSTR("Backslash");
        case InputButton::Backspace:            return JSTR("Backspace");
        case InputButton::Enter:                return JSTR("Enter");
        case InputButton::Tab:                  return JSTR("Tab");
        case InputButton::CapsLock:             return JSTR("CapsLock");
        case InputButton::ShiftLeft:            return JSTR("ShiftLeft");
        case InputButton::ShiftRight:           return JSTR("ShiftRight");
        case InputButton::ControlLeft:          return JSTR("ControlLeft");
        case InputButton::ControlRight:         return JSTR("ControlRight");
        case InputButton::AltLeft:              return JSTR("AltLeft");
        case InputButton::AltRight:             return JSTR("AltRight");
        case InputButton::SuperLeft:            return JSTR("SuperLeft");
        case InputButton::SuperRight:           return JSTR("SuperRight");
        case InputButton::Space:                return JSTR("Space");
        case InputButton::Escape:               return JSTR("Escape");
        case InputButton::F1:                   return JSTR("F1");
        case InputButton::F2:                   return JSTR("F2");
        case InputButton::F3:                   return JSTR("F3");
        case InputButton::F4:                   return JSTR("F4");
        case InputButton::F5:                   return JSTR("F5");
        case InputButton::F6:                   return JSTR("F6");
        case InputButton::F7:                   return JSTR("F7");
        case InputButton::F8:                   return JSTR("F8");
        case InputButton::F9:                   return JSTR("F9");
        case InputButton::F10:                  return JSTR("F10");
        case InputButton::F11:                  return JSTR("F11");
        case InputButton::F12:                  return JSTR("F12");
        case InputButton::ArrowRight:           return JSTR("ArrowRight");
        case InputButton::ArrowLeft:            return JSTR("ArrowLeft");
        case InputButton::ArrowDown:            return JSTR("ArrowDown");
        case InputButton::ArrowUp:              return JSTR("ArrowUp");
        case InputButton::Insert:               return JSTR("Insert");
        case InputButton::Delete:               return JSTR("Delete");
        case InputButton::Home:                 return JSTR("Home");
        case InputButton::End:                  return JSTR("End");
        case InputButton::PageUp:               return JSTR("PageUp");
        case InputButton::PageDown:             return JSTR("PageDown");
        case InputButton::PrintScreen:          return JSTR("PrintScreen");
        case InputButton::ScrollLock:           return JSTR("ScrollLock");
        case InputButton::Pause:                return JSTR("Pause");
        case InputButton::NumLock:              return JSTR("NumLock");
        case InputButton::NumPad0:              return JSTR("NumPad0");
        case InputButton::NumPad1:              return JSTR("NumPad1");
        case InputButton::NumPad2:              return JSTR("NumPad2");
        case InputButton::NumPad3:              return JSTR("NumPad3");
        case InputButton::NumPad4:              return JSTR("NumPad4");
        case InputButton::NumPad5:              return JSTR("NumPad5");
        case InputButton::NumPad6:              return JSTR("NumPad6");
        case InputButton::NumPad7:              return JSTR("NumPad7");
        case InputButton::NumPad8:              return JSTR("NumPad8");
        case InputButton::NumPad9:              return JSTR("NumPad9");
        case InputButton::NumPadDot:            return JSTR("NumPadDot");
        case InputButton::NumPadDivide:         return JSTR("NumPadDivide");
        case InputButton::NumPadMultiply:       return JSTR("NumPadMultiply");
        case InputButton::NumPadMinus:          return JSTR("NumPadMinus");
        case InputButton::NumPadPlus:           return JSTR("NumPadPlus");
        case InputButton::NumPadEnter:          return JSTR("NumPadEnter");
        case InputButton::MouseLeft:            return JSTR("MouseLeft");
        case InputButton::MouseRight:           return JSTR("MouseRight");
        case InputButton::MouseWheel:           return JSTR("MouseWheel");
        case InputButton::MouseButton4:         return JSTR("MouseButton4");
        case InputButton::MouseButton5:         return JSTR("MouseButton5");
        case InputButton::MouseButton6:         return JSTR("MouseButton6");
        case InputButton::MouseButton7:         return JSTR("MouseButton7");
        case InputButton::MouseButton8:         return JSTR("MouseButton8");
        case InputButton::GamepadA:             return JSTR("GamepadA");
        case InputButton::GamepadB:             return JSTR("GamepadB");
        case InputButton::GamepadX:             return JSTR("GamepadX");
        case InputButton::GamepadY:             return JSTR("GamepadY");
        case InputButton::GamepadBumperLeft:    return JSTR("GamepadBumperLeft");
        case InputButton::GamepadBumperRight:   return JSTR("GamepadBumperRight");
        case InputButton::GamepadTriggerLeft:   return JSTR("GamepadTriggerLeft");
        case InputButton::GamepadTriggerRight:  return JSTR("GamepadTriggerRight");
        case InputButton::GamepadBack:          return JSTR("GamepadBack");
        case InputButton::GamepadStart:         return JSTR("GamepadStart");
        case InputButton::GamepadGuide:         return JSTR("GamepadGuide");
        case InputButton::GamepadDPadUp:        return JSTR("GamepadDPadUp");
        case InputButton::GamepadDPadRight:     return JSTR("GamepadDPadRight");
        case InputButton::GamepadDPadDown:      return JSTR("GamepadDPadDown");
        case InputButton::GamepadDPadLeft:      return JSTR("GamepadDPadLeft");
        default: ;
        }
        return JSTR("NONE");
    }
    constexpr const char* InputAxisToString(const InputAxis axis)
    {
        switch (axis)
        {
        case InputAxis::Mouse2D: return JSTR("Mouse2D");
        case InputAxis::MouseWheel: return JSTR("MouseWheel");
        case InputAxis::GamepadTriggerLeft: return JSTR("GamepadTriggerLeft");
        case InputAxis::GamepadTriggerRight: return JSTR("GamepadTriggerRight");
        case InputAxis::GamepadStick2DLeft: return JSTR("GamepadStick2DLeft");
        case InputAxis::GamepadStick2DRight: return JSTR("GamepadStick2DRight");
        default: ;
        }
        return JSTR("NONE");
    }
}

JUTILS_LOG_FORMATTER(JumaRenderEngine::InputDeviceType, JumaRenderEngine::InputDeviceTypeToString);
JUTILS_LOG_FORMATTER(JumaRenderEngine::InputButton, JumaRenderEngine::InputButtonToString);
JUTILS_LOG_FORMATTER(JumaRenderEngine::InputAxis, JumaRenderEngine::InputAxisToString);
