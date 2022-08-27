// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL)

#include "Material_OpenGL.h"

#include <GL/glew.h>

#include "RenderTarget_OpenGL.h"
#include "Shader_OpenGL.h"
#include "Texture_OpenGL.h"
#include "../../include/JumaRE/RenderEngine.h"

namespace JumaRenderEngine
{
    Material_OpenGL::~Material_OpenGL()
    {
        clearOpenGL();
    }

    bool Material_OpenGL::initInternal()
    {
        const jmap<uint32, ShaderUniformBufferDescription>& uniformBufferDescriptions = getShader()->getUniformBufferDescriptions();
        if (!uniformBufferDescriptions.isEmpty())
        {
            jarray<uint32> uniformBuffers(uniformBufferDescriptions.getSize(), 0);
            glGenBuffers(uniformBuffers.getSize(), uniformBuffers.getData());
            for (const auto& uniformBufferDescription : uniformBufferDescriptions)
            {
                const uint32 bufferIndex = m_UniformBufferIndices[uniformBufferDescription.key] = uniformBuffers.getLast();
                uniformBuffers.removeLast();

                glBindBuffer(GL_UNIFORM_BUFFER, bufferIndex);
                glBufferData(GL_UNIFORM_BUFFER, uniformBufferDescription.value.size, nullptr, GL_DYNAMIC_DRAW);
            }
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        return true;
    }

    void Material_OpenGL::clearAsset()
    {
        clearOpenGL();
        Super::clearAsset();
    }
    void Material_OpenGL::clearOpenGL()
    {
        for (const auto& buffer : m_UniformBufferIndices)
        {
            glDeleteBuffers(1, &buffer.value);
        }
        m_UniformBufferIndices.clear();
    }

    bool Material_OpenGL::bindMaterial()
    {
        if (!getShader<Shader_OpenGL>()->activateShader())
        {
            return false;
        }

        updateUniformData();
        for (const auto& uniformBuffer : m_UniformBufferIndices)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, uniformBuffer.key, uniformBuffer.value);
        }

        const MaterialProperties& properties = getMaterialProperties();
        if (properties.depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
        if (properties.stencilEnabled)
        {
            glEnable(GL_STENCIL_TEST);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
        glPolygonMode(GL_FRONT_AND_BACK, properties.wireframe ? GL_LINE : GL_FILL);
        glCullFace(properties.cullBackFaces ? GL_BACK : GL_FRONT);
        if (properties.blendEnabled)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            glDisable(GL_BLEND);
        }

        return true;
    }
    void Material_OpenGL::updateUniformData()
    {
        const Texture_OpenGL* defaultTexture = dynamic_cast<const Texture_OpenGL*>(getRenderEngine()->getDefaultTexture());
        const jset<jstringID>& notUpdatedParams = getNotUpdatedParams();
        const MaterialParamsStorage& materialParams = getMaterialParams();
        for (const auto& uniform : getShader()->getUniforms())
        {
            if (uniform.value.type == ShaderUniformType::Texture)
            {
                ShaderUniformInfo<ShaderUniformType::Texture>::value_type value = nullptr;
                materialParams.getValue<ShaderUniformType::Texture>(uniform.key, value);

                const Texture_OpenGL* texture = dynamic_cast<Texture_OpenGL*>(value);
                if (texture != nullptr)
                {
                    texture->bindToShader(uniform.value.shaderLocation);
                }
                else
                {
                    const RenderTarget_OpenGL* renderTarget = dynamic_cast<RenderTarget_OpenGL*>(value);
                    if (renderTarget != nullptr)
                    {
                        renderTarget->bindToShader(uniform.value.shaderLocation);
                    }
                    else if (defaultTexture != nullptr)
                    {
                        defaultTexture->bindToShader(uniform.value.shaderLocation);
                    }
                    else
                    {
                        throw std::exception("Invalid default texture");
                    }
                }
            }
            else if (notUpdatedParams.contains(uniform.key))
            {
                switch (uniform.value.type)
                {
                case ShaderUniformType::Float:
                    {
                        ShaderUniformInfo<ShaderUniformType::Float>::value_type value;
                        if (materialParams.getValue<ShaderUniformType::Float>(uniform.key, value))
                        {
                            glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.value.shaderLocation]);
                            glBufferSubData(GL_UNIFORM_BUFFER, uniform.value.shaderBlockOffset, sizeof(value), &value);
                        }
                    }
                    break;
                case ShaderUniformType::Vec2:
                    {
                        ShaderUniformInfo<ShaderUniformType::Vec2>::value_type value;
                        if (materialParams.getValue<ShaderUniformType::Vec2>(uniform.key, value))
                        {
                            glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.value.shaderLocation]);
                            glBufferSubData(GL_UNIFORM_BUFFER, uniform.value.shaderBlockOffset, sizeof(value), &value[0]);
                        }
                    }
                    break;
                case ShaderUniformType::Vec4:
                    {
                        ShaderUniformInfo<ShaderUniformType::Vec4>::value_type value;
                        if (materialParams.getValue<ShaderUniformType::Vec4>(uniform.key, value))
                        {
                            glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.value.shaderLocation]);
                            glBufferSubData(GL_UNIFORM_BUFFER, uniform.value.shaderBlockOffset, sizeof(value), &value[0]);
                        }
                    }
                    break;
                case ShaderUniformType::Mat4:
                    {
                        ShaderUniformInfo<ShaderUniformType::Mat4>::value_type value;
                        if (materialParams.getValue<ShaderUniformType::Mat4>(uniform.key, value))
                        {
                            glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.value.shaderLocation]);
                            glBufferSubData(GL_UNIFORM_BUFFER, uniform.value.shaderBlockOffset, sizeof(value), &value[0][0]);
                        }
                    }
                    break;

                default: ;
                }
            }  
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        clearParamsForUpdate();
    }

    void Material_OpenGL::unbindMaterial()
    {
        for (const auto& uniformBuffer : m_UniformBufferIndices)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, uniformBuffer.key, 0);
        }
        for (const auto& uniform : getShader()->getUniforms())
        {
            if (uniform.value.type == ShaderUniformType::Texture)
            {
                Texture_OpenGL::unbindTexture(uniform.value.shaderLocation);
            }
        }
        Shader_OpenGL::deactivateAnyShader();
    }
}

#endif
