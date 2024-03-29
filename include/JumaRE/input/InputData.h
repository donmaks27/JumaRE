﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../input/InputButtons.h"

#include <jutils/jmap.h>
#include <jutils/math/vector2.h>

namespace JumaRenderEngine
{
    class WindowController;

    class InputData
    {
        friend WindowController;

    public:
        InputData() = default;

        bool getButtonState(InputDevice device, InputButton button, InputButtonAction* outAction, input_mods_type* outMods = nullptr) const;
        bool getAxisState(InputDevice device, InputAxis axis, float* outValue, input_mods_type* outMods = nullptr) const;
        bool getAxisState2D(InputDevice device, InputAxis axis, math::vector2* outValue, input_mods_type* outMods = nullptr) const;

        bool setButtonState(InputDevice device, InputButton button, InputButtonAction action, input_mods_type mods = 0);
        bool setAxisState(InputDevice device, InputAxis axis, const math::vector2& value, input_mods_type mods = 0);
        bool setAxisState(InputDevice device, InputAxis axis, float value, input_mods_type mods = 0);
        bool setAxisState2D(InputDevice device, InputAxis axis, const math::vector2& value, input_mods_type mods = 0);

    private:

        struct InputButtonDataKey
        {
            InputButton button = InputButton::NONE;
            InputDevice device = InputDevice::NONE;

            constexpr bool operator<(const InputButtonDataKey& key) const { return button != key.button ? button < key.button : device < key.device; }
        };
        struct InputButtonData
        {
            InputButtonAction action = InputButtonAction::Release;
            input_mods_type mods = 0;
        };

        struct InputAxisDataKey
        {
            InputAxis axis = InputAxis::NONE;
            InputDevice device = InputDevice::NONE;

            constexpr bool operator<(const InputAxisDataKey& key) const { return axis != key.axis ? axis < key.axis : device < key.device; }
        };
        struct InputAxisData
        {
            math::vector2 value = { 0.0f, 0.0f };
            input_mods_type mods = 0;
        };

        jmap<InputButtonDataKey, InputButtonData> m_ButtonStates;
        jmap<InputAxisDataKey, InputAxisData> m_AxisStates;
    };
}
