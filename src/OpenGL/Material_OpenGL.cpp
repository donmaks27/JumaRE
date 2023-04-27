// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL)

#include "Material_OpenGL.h"

#include <GL/glew.h>

#include "RenderTarget_OpenGL.h"
#include "Shader_OpenGL.h"
#include "Texture_OpenGL.h"
#include "JumaRE/RenderEngine.h"
#include "JumaRE/RenderOptions.h"

namespace JumaRenderEngine
{
    Material_OpenGL::~Material_OpenGL()
    {
        clearOpenGL();
    }

    bool Material_OpenGL::initInternal()
    {
        if (getShader()->getUniformBufferDescriptions().isEmpty())
        {
            m_MaterialCreated = true;
            return true;
        }

        m_CreateTaskActive = true;
        jasync_task* task = new CreateMaterialTask(this);
        if (!getRenderEngine()->getAsyncAssetTaksQueue().addTask(task))
        {
            JUTILS_LOG(error, JSTR("Failed to start async uniform buffers creation task"));
            delete task;
            return false;
        }
        return true;
    }
    void Material_OpenGL::createUniformBuffers()
    {
        const Shader* shader = getShader();
        if (shader == nullptr)
        {
            return;
        }

        const jmap<uint32, ShaderUniformBufferDescription>& uniformBufferDescriptions = shader->getUniformBufferDescriptions();
        jarray<uint32> uniformBuffers(static_cast<int32>(uniformBufferDescriptions.getSize()), 0);
        glGenBuffers(uniformBuffers.getSize(), uniformBuffers.getData());
        for (const auto& [bufferID, bufferDescription] : uniformBufferDescriptions)
        {
            const uint32 bufferIndex = uniformBuffers.getLast();
            m_UniformBufferIndices.add(bufferID, bufferIndex);
            uniformBuffers.removeLast();

            glBindBuffer(GL_UNIFORM_BUFFER, bufferIndex);
            glBufferData(GL_UNIFORM_BUFFER, bufferDescription.size, nullptr, GL_DYNAMIC_DRAW);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void Material_OpenGL::onClearAsset()
    {
        clearOpenGL();
        Super::onClearAsset();
    }
    void Material_OpenGL::clearOpenGL()
    {
        for (const auto& buffer : m_UniformBufferIndices.values())
        {
            glDeleteBuffers(1, &buffer);
        }
        m_UniformBufferIndices.clear();
    }

    bool Material_OpenGL::bindMaterial(const RenderOptions* renderOptions)
    {
        const Shader_OpenGL* shader = getShader<Shader_OpenGL>();
        if (!m_MaterialCreated)
        {
            if (m_CreateTaskActive)
            {
                return false;
            }
            m_MaterialCreated = true;
        }

        if (!shader->activateShader())
        {
            return false;
        }

        updateUniformData();
        for (const auto& [bufferID, bufferIndex] : m_UniformBufferIndices)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, bufferID, bufferIndex);
        }

        MaterialProperties properties = getMaterialProperties();
        properties.depthEnabled &= renderOptions->renderStageProperties.depthEnabled;
        if (properties.depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
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
        for (const auto& [uniformID, uniform] : getShader()->getUniforms())
        {
            if (uniform.type == ShaderUniformType::Texture)
            {
                ShaderUniformInfo<ShaderUniformType::Texture>::value_type value = nullptr;
                materialParams.getValue<ShaderUniformType::Texture>(uniformID, value);

                const Texture_OpenGL* texture = dynamic_cast<Texture_OpenGL*>(value);
                if (texture != nullptr)
                {
                    texture->bindToShader(uniform.shaderLocation);
                }
                else
                {
                    const RenderTarget_OpenGL* renderTarget = dynamic_cast<RenderTarget_OpenGL*>(value);
                    if (renderTarget != nullptr)
                    {
                        renderTarget->bindToShader(uniform.shaderLocation);
                    }
                    else if (defaultTexture != nullptr)
                    {
                        defaultTexture->bindToShader(uniform.shaderLocation);
                    }
                    else
                    {
                        throw std::logic_error("Invalid default texture");
                    }
                }
            }
            else if (notUpdatedParams.contains(uniformID))
            {
                switch (uniform.type)
                {
                case ShaderUniformType::Float:
                    {
                        ShaderUniformInfo<ShaderUniformType::Float>::value_type value;
                        if (materialParams.getValue<ShaderUniformType::Float>(uniformID, value))
                        {
                            glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.shaderLocation]);
                            glBufferSubData(GL_UNIFORM_BUFFER, uniform.shaderBlockOffset, sizeof(value), &value);
                        }
                    }
                    break;
                case ShaderUniformType::Vec2:
                    {
                        ShaderUniformInfo<ShaderUniformType::Vec2>::value_type value;
                        if (materialParams.getValue<ShaderUniformType::Vec2>(uniformID, value))
                        {
                            glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.shaderLocation]);
                            glBufferSubData(GL_UNIFORM_BUFFER, uniform.shaderBlockOffset, sizeof(value), &value[0]);
                        }
                    }
                    break;
                case ShaderUniformType::Vec4:
                    {
                        ShaderUniformInfo<ShaderUniformType::Vec4>::value_type value;
                        if (materialParams.getValue<ShaderUniformType::Vec4>(uniformID, value))
                        {
                            glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.shaderLocation]);
                            glBufferSubData(GL_UNIFORM_BUFFER, uniform.shaderBlockOffset, sizeof(value), &value[0]);
                        }
                    }
                    break;
                case ShaderUniformType::Mat4:
                    {
                        ShaderUniformInfo<ShaderUniformType::Mat4>::value_type value;
                        if (materialParams.getValue<ShaderUniformType::Mat4>(uniformID, value))
                        {
                            glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.shaderLocation]);
                            glBufferSubData(GL_UNIFORM_BUFFER, uniform.shaderBlockOffset, sizeof(value), &value[0][0]);
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
        for (const auto& uniformBuffer : m_UniformBufferIndices.keys())
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, uniformBuffer, 0);
        }
        for (const auto& uniform : getShader()->getUniforms().values())
        {
            if (uniform.type == ShaderUniformType::Texture)
            {
                Texture_OpenGL::unbindTexture(uniform.shaderLocation);
            }
        }
        Shader_OpenGL::deactivateAnyShader();
    }
}

#endif
