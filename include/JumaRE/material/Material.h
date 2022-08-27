// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"
#include "../RenderEngineAsset.h"

#include <jutils/jset.h>

#include "MaterialParamsStorage.h"
#include "MaterialProperties.h"

namespace JumaRenderEngine
{
    class Shader;

    class Material : public RenderEngineAsset
    {
        friend RenderEngine;

    public:
        Material() = default;
        virtual ~Material() override;

        Shader* getShader() const { return m_Shader; }
        const MaterialProperties& getMaterialProperties() const { return m_Properties; }
        const MaterialParamsStorage& getMaterialParams() const { return m_MaterialParams; }

        template<ShaderUniformType Type>
        bool setParamValue(const jstringID& name, const typename ShaderUniformInfo<Type>::value_type& value)
        {
            if (checkParamType(name, Type) && m_MaterialParams.setValue<Type>(name, value))
            {
                m_MaterialParamsForUpdate.add(name);
                return true;
            }
            return false;
        }
        bool resetParamValue(const jstringID& name);
        template<ShaderUniformType Type>
        bool getValue(const jstringID& name, typename ShaderUniformInfo<Type>::value_type& outValue) const
        {
            return checkParamType(name, Type) && m_MaterialParams.getValue<Type>(name, outValue);
        }

    protected:
        
        bool init(Shader* shader);

        virtual bool initInternal() = 0;
        virtual void clearAsset() override { clearData(); }

        template<typename T, TEMPLATE_ENABLE(is_base<Shader, T>)>
        T* getShader() const { return dynamic_cast<T*>(getShader()); }
        
        const jset<jstringID>& getNotUpdatedParams() const { return m_MaterialParamsForUpdate; }
        void clearParamsForUpdate() { m_MaterialParamsForUpdate.clear(); }

    private:

        Shader* m_Shader = nullptr;
        MaterialProperties m_Properties;
        MaterialParamsStorage m_MaterialParams;
        jset<jstringID> m_MaterialParamsForUpdate;


        void clearData();

        bool checkParamType(const jstringID& name, ShaderUniformType type) const;
    };
}
