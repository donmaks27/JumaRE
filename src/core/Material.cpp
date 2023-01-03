// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/material/Material.h"

#include "JumaRE/material/Shader.h"

namespace JumaRenderEngine
{
    Material::~Material()
    {
        clearData();
    }

    bool Material::init(Shader* shader, const bool templateMaterial)
    {
        if (shader == nullptr)
        {
            JUTILS_LOG(error, JSTR("Invalid shader"));
            return false;
        }

        m_Shader = shader;
        for (const auto& uniform : m_Shader->getUniforms())
        {
            m_MaterialParams.setDefaultValue(uniform.key, uniform.value.type);
            m_MaterialParamsForUpdate.add(uniform.key);
        }
        m_TemplateMaterial = templateMaterial;

        if (!initInternal())
        {
            JUTILS_LOG(error, JSTR("Failed to initizlie material"));
            clearData();
            return false;
        }
        return true;
    }

    void Material::clearData()
    {
        m_MaterialParams.clear();
        m_Shader = nullptr;
    }

    bool Material::checkParamType(const jstringID& name, const ShaderUniformType type) const
    {
        const ShaderUniform* uniform = m_Shader->getUniforms().find(name);
        return (uniform != nullptr) && (uniform->type == type);
    }
    bool Material::resetParamValue(const jstringID& name)
    {
        const ShaderUniform* uniform = m_Shader->getUniforms().find(name);
        if ((uniform != nullptr) && m_MaterialParams.setDefaultValue(name, uniform->type))
        {
            m_MaterialParamsForUpdate.add(name);
            return true;
        }
        return false;
    }
}
