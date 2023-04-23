// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/input/InputData.h"

namespace JumaRenderEngine
{
    bool InputData::getButtonState(const InputDevice device, const InputButton button, InputButtonAction* outAction, 
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
    bool InputData::getAxisState(const InputDevice device, const InputAxis axis, float* outValue, input_mods_type* outMods) const
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
    bool InputData::getAxisState2D(const InputDevice device, const InputAxis axis, math::vector2* outValue, input_mods_type* outMods) const
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

    bool InputData::setButtonState(const InputDevice device, const InputButton button, const InputButtonAction action, 
        const input_mods_type mods)
    {
        if (!IsInputDeviceContainsInputButton(device, button))
        {
            return false;
        }

        const InputButtonDataKey key = { button, device };
        InputButtonData* data = m_ButtonStates.find(key);
        if (data == nullptr)
        {
            m_ButtonStates.add(key, { action, mods });
            return true;
        }
        if ((action == InputButtonAction::Repeate) || (data->action != action))
        {
            *data = { action, mods };
            return true;
        }
        return false;
    }
    bool InputData::setAxisState(const InputDevice device, const InputAxis axis, const math::vector2& value, const input_mods_type mods)
    {
        return IsInputAxis2D(axis) ? setAxisState2D(device, axis, value, mods) : setAxisState(device, axis, value.x, mods);
    }
    bool InputData::setAxisState(const InputDevice device, const InputAxis axis, const float value, const input_mods_type mods)
    {
        if (!IsInputDeviceContainsInputAxis(device, axis) && !IsInputAxis2D(axis))
        {
            return false;
        }

        const InputAxisDataKey key = { axis, device };
        InputAxisData* data = m_AxisStates.find(key);
        if (data == nullptr)
        {
            m_AxisStates.add(key, { { value, 0.0f }, mods });
        }
        else if (!math::isNearlyZero(value) || !math::isNearlyZero(data->value.x))
        {
            *data = { { value, 0.0f }, mods };
        }
        return true;
    }
    bool InputData::setAxisState2D(const InputDevice device, const InputAxis axis, const math::vector2& value, const input_mods_type mods)
    {
        if (!IsInputDeviceContainsInputAxis(device, axis) && IsInputAxis2D(axis))
        {
            return false;
        }

        const InputAxisDataKey key = { axis, device };
        InputAxisData* data = m_AxisStates.find(key);
        if (data == nullptr)
        {
            m_AxisStates.add(key, { value, mods });
        }
        else if (!math::isNearlyZero(value.x) || !math::isNearlyZero(value.y) || 
                 !math::isNearlyZero(data->value.x) || !math::isNearlyZero(data->value.y))
        {
            *data = { value, mods };
        }
        return true;
    }
}
