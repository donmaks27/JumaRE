// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

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

        using Super = RenderEngineAsset;

    public:
        Material() : Super(RenderEngineAssetType::Material) {}
        virtual ~Material() override;

        Shader* getShader() const { return m_Shader; }
        const MaterialProperties& getMaterialProperties() const { return m_Properties; }
        const MaterialParamsStorage& getMaterialParams() const { return m_MaterialParams; }
        bool isTemplateMaterial() const { return m_TemplateMaterial; }

        template<ShaderUniformType Type>
        bool setParamValue(const jstringID& name, const typename ShaderUniformInfo<Type>::value_type& value)
        {
            if (!checkParamType(name, Type) || !m_MaterialParams.setValue<Type>(name, value))
            {
	            return false;
            }
            if (!isTemplateMaterial())
            {
                m_MaterialParamsForUpdate.add(name);
            }
            return true;
        }
        bool resetParamValue(const jstringID& name);
        template<ShaderUniformType Type>
        bool getParamValue(const jstringID& name, typename ShaderUniformInfo<Type>::value_type& outValue) const
        {
            return checkParamType(name, Type) && m_MaterialParams.getValue<Type>(name, outValue);
        }

    protected:

        virtual bool initInternal() = 0;
        virtual void onClearAsset() override;

        template<typename T, TEMPLATE_ENABLE(is_base<Shader, T>)>
        T* getShader() const { return dynamic_cast<T*>(getShader()); }
        
        const jset<jstringID>& getNotUpdatedParams() const { return m_MaterialParamsForUpdate; }
        void clearParamsForUpdate() { m_MaterialParamsForUpdate.clear(); }

    private:

        Shader* m_Shader = nullptr;
        MaterialProperties m_Properties;
        MaterialParamsStorage m_MaterialParams;

        jset<jstringID> m_MaterialParamsForUpdate;

        bool m_TemplateMaterial = false;


        bool init(Shader* shader);

        void clearData();

        bool checkParamType(const jstringID& name, ShaderUniformType type) const;
    };
}
