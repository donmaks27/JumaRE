// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/material/Shader.h"

#include "JumaRE/material/ShaderUniformInfo.h"

namespace JumaRenderEngine
{
    Shader::~Shader()
    {
        clearData();
    }

    bool Shader::init(const ShaderCreateInfo& createInfo)
    {
        m_VertexComponents = createInfo.vertexComponents;

        m_ShaderUniforms = createInfo.uniforms;
        for (const auto& uniform : m_ShaderUniforms.values())
        {
            const uint32 size = GetShaderUniformValueSize(uniform.type);
            if (size == 0)
            {
                continue;
            }

            ShaderUniformBufferDescription& uniformBuffer = m_CachedUniformBufferDescriptions[uniform.shaderLocation];
            uniformBuffer.size = math::max(uniformBuffer.size, uniform.shaderBlockOffset + size);
            uniformBuffer.shaderStages |= uniform.shaderStages;
        }

        if (!initInternal(createInfo.fileNames))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize shader"));
            clearData();
            return false;
        }
        return true;
    }

    void Shader::onClearAsset()
    {
	    clearData();
        Super::onClearAsset();
    }
    void Shader::clearData()
    {
        m_CachedUniformBufferDescriptions.clear();
        m_ShaderUniforms.clear();
        m_VertexComponents.clear();
    }
}
