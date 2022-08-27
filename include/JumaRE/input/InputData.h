// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../input/InputButtons.h"

#include <jutils/jmap.h>
#include <jutils/math/vector2.h>
#include <jutils/math/math_vector.h>

namespace JumaRenderEngine
{
    class InputData
    {
    public:
        InputData() = default;
        ~InputData() = default;

        bool getButtonState(InputDeviceType device, InputButton button, InputButtonAction* outAction, input_mods_type* outMods = nullptr) const;
        bool getAxisState(InputDeviceType device, InputAxis axis, float* outValue, input_mods_type* outMods = nullptr) const;
        bool getAxisState2D(InputDeviceType device, InputAxis axis, math::vector2* outValue, input_mods_type* outMods = nullptr) const;

        bool setButtonState(InputDeviceType device, InputButton button, InputButtonAction action, input_mods_type mods = 0);
        bool setAxisState(InputDeviceType device, InputAxis axis, const math::vector2& value, input_mods_type mods = 0);
        bool setAxisState(InputDeviceType device, InputAxis axis, float value, input_mods_type mods = 0);
        bool setAxisState2D(InputDeviceType device, InputAxis axis, const math::vector2& value, input_mods_type mods = 0);

    private:

        struct InputButtonDataKey
        {
            InputButton button = InputButton::NONE;
            InputDeviceType device = InputDeviceType::NONE;

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
            InputDeviceType device = InputDeviceType::NONE;

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

    inline bool InputData::getButtonState(const InputDeviceType device, const InputButton button, InputButtonAction* outAction, 
        input_mods_type* outMods) const
    {
        if (!IsInputDeviceContainsInputButton(device, button))
        {
            return false;
        }
        const InputButtonData* data = m_ButtonStates.find({ button, device });
        if (outAction != nullptr)
        {
            *outAction = data != nullptr ? data->action : InputButtonAction::Release;
        }
        if (outMods != nullptr)
        {
            *outMods = data != nullptr ? data->mods : 0;
        }
        return true;
    }
    inline bool InputData::getAxisState(const InputDeviceType device, const InputAxis axis, float* outValue, input_mods_type* outMods) const
    {
        if (!IsInputDeviceContainsInputAxis(device, axis) || IsInputAxis2D(axis))
        {
            return false;
        }
        const InputAxisData* data = m_AxisStates.find({ axis, device });
        if (outValue != nullptr)
        {
            *outValue = data != nullptr ? data->value.x : 0.0f;
        }
        if (outMods != nullptr)
        {
            *outMods = data != nullptr ? data->mods : 0;
        }
        return true;
    }
    inline bool InputData::getAxisState2D(const InputDeviceType device, const InputAxis axis, math::vector2* outValue, input_mods_type* outMods) const
    {
        if (!IsInputDeviceContainsInputAxis(device, axis) || !IsInputAxis2D(axis))
        {
            return false;
        }
        const InputAxisData* data = m_AxisStates.find({ axis, device });
        if (outValue != nullptr)
        {
            *outValue = data != nullptr ? data->value : math::vector2(0.0f, 0.0f);
        }
        if (outMods != nullptr)
        {
            *outMods = data != nullptr ? data->mods : 0;
        }
        return true;
    }

    inline bool InputData::setButtonState(const InputDeviceType device, const InputButton button, const InputButtonAction action, 
        const input_mods_type mods)
    {
        if (!IsInputDeviceContainsInputButton(device, button))
        {
            return false;
        }
        m_ButtonStates.add({ button, device }, { action, mods });
        return true;
    }
    inline bool InputData::setAxisState(const InputDeviceType device, const InputAxis axis, const math::vector2& value, const input_mods_type mods)
    {
        if (!IsInputDeviceContainsInputAxis(device, axis))
        {
            return false;
        }
        m_AxisStates.add({ axis, device }, { value, mods });
        return true;
    }
    inline bool InputData::setAxisState(const InputDeviceType device, const InputAxis axis, const float value, const input_mods_type mods)
    {
        if (!IsInputDeviceContainsInputAxis(device, axis) && !IsInputAxis2D(axis))
        {
            return false;
        }
        m_AxisStates.add({ axis, device }, { { value, 0.0f }, mods });
        return true;
    }
    inline bool InputData::setAxisState2D(const InputDeviceType device, const InputAxis axis, const math::vector2& value, 
        const input_mods_type mods)
    {
        if (!IsInputDeviceContainsInputAxis(device, axis) && IsInputAxis2D(axis))
        {
            return false;
        }
        m_AxisStates.add({ axis, device }, { value, mods });
        return true;
    }
}
