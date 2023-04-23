// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

#include <jutils/jmap.h>
#include <jutils/jstringID.h>
#include <jutils/math/math_matrix.h>

#include "ShaderUniformInfo.h"

namespace JumaRenderEngine
{
    class MaterialParamsStorage final
    {
    public:
        MaterialParamsStorage() = default;
        ~MaterialParamsStorage();

        template<ShaderUniformType Type>
        bool setValue(const jstringID& name, const typename ShaderUniformInfo<Type>::value_type& value)
        {
            return (name != jstringID_NONE) && this->setValueInternal<Type>(name, value);
        }
        bool setDefaultValue(const jstringID& name, ShaderUniformType type);
        bool removeValue(const jstringID& name, ShaderUniformType type);

        template<ShaderUniformType Type>
        bool getValue(const jstringID& name, typename ShaderUniformInfo<Type>::value_type& outValue) const
        {
            const typename ShaderUniformInfo<Type>::value_type* value = findValue<Type>(name);
            if (value == nullptr)
            {
                return false;
            }
            outValue = *value;
            return true;
        }
        bool contains(const jstringID& name, ShaderUniformType type) const;
        
        void clear();

    private:

        template<ShaderUniformType Type>
        using material_params_map = jmap<jstringID, typename ShaderUniformInfo<Type>::value_type>;

        // TODO: Replace with global objects pool in render engine, it took too much memory for each material
        material_params_map<ShaderUniformType::Float> m_MaterialParams_Float;
        material_params_map<ShaderUniformType::Vec2> m_MaterialParams_Vec2;
        material_params_map<ShaderUniformType::Vec4> m_MaterialParams_Vec4;
        material_params_map<ShaderUniformType::Mat4> m_MaterialParams_Mat4;
        material_params_map<ShaderUniformType::Texture> m_MaterialParams_Texture;


        template<ShaderUniformType Type>
        bool setValueInternal(const jstringID& name, const typename ShaderUniformInfo<Type>::value_type& value) { return false; }
        template<>
        bool setValueInternal<ShaderUniformType::Float>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Float>::value_type& value)
        {
            ShaderUniformInfo<ShaderUniformType::Float>::value_type* valuePtr = m_MaterialParams_Float.find(name);
            if (valuePtr == nullptr)
            {
                m_MaterialParams_Float.add(name, value);
                return true;
            }
            if (!math::isEqual(value, *valuePtr))
            {
                *valuePtr = value;
                return true;
            }
            return false;
        }
        template<>
        bool setValueInternal<ShaderUniformType::Vec2>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Vec2>::value_type& value)
        {
            ShaderUniformInfo<ShaderUniformType::Vec2>::value_type* valuePtr = m_MaterialParams_Vec2.find(name);
            if (valuePtr == nullptr)
            {
                m_MaterialParams_Vec2.add(name, value);
                return true;
            }
            if (value != *valuePtr)
            {
                *valuePtr = value;
                return true;
            }
            return false;
        }
        template<>
        bool setValueInternal<ShaderUniformType::Vec4>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Vec4>::value_type& value)
        {
            ShaderUniformInfo<ShaderUniformType::Vec4>::value_type* valuePtr = m_MaterialParams_Vec4.find(name);
            if (valuePtr == nullptr)
            {
                m_MaterialParams_Vec4.add(name, value);
                return true;
            }
            if (value != *valuePtr)
            {
                *valuePtr = value;
                return true;
            }
            return false;
        }
        template<>
        bool setValueInternal<ShaderUniformType::Mat4>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Mat4>::value_type& value)
        {
            ShaderUniformInfo<ShaderUniformType::Mat4>::value_type* valuePtr = m_MaterialParams_Mat4.find(name);
            if (valuePtr == nullptr)
            {
                m_MaterialParams_Mat4.add(name, value);
                return true;
            }
            if (value != *valuePtr)
            {
                *valuePtr = value;
                return true;
            }
            return false;
        }
        template<>
        bool setValueInternal<ShaderUniformType::Texture>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Texture>::value_type& value)
        {
            ShaderUniformInfo<ShaderUniformType::Texture>::value_type* valuePtr = m_MaterialParams_Texture.find(name);
            if (valuePtr == nullptr)
            {
                m_MaterialParams_Texture.add(name, value);
                return true;
            }
            if (value != *valuePtr)
            {
                *valuePtr = value;
                return true;
            }
            return false;
        }

        template<ShaderUniformType Type>
        const typename ShaderUniformInfo<Type>::value_type* findValue(const jstringID& name) const { return nullptr; }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Float>::value_type* findValue<ShaderUniformType::Float>(const jstringID& name) const { return m_MaterialParams_Float.find(name); }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Vec2>::value_type* findValue<ShaderUniformType::Vec2>(const jstringID& name) const { return m_MaterialParams_Vec2.find(name); }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Vec4>::value_type* findValue<ShaderUniformType::Vec4>(const jstringID& name) const { return m_MaterialParams_Vec4.find(name); }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Mat4>::value_type* findValue<ShaderUniformType::Mat4>(const jstringID& name) const { return m_MaterialParams_Mat4.find(name); }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Texture>::value_type* findValue<ShaderUniformType::Texture>(const jstringID& name) const { return m_MaterialParams_Texture.find(name); }
    };
}
