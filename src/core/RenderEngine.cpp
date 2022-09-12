﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/RenderEngine.h"

#include "../../include/JumaRE/RenderPipeline.h"
#include "../../include/JumaRE/RenderTarget.h"
#include "../../include/JumaRE/material/Material.h"
#include "../../include/JumaRE/material/Shader.h"
#include "../../include/JumaRE/texture/Texture.h"
#include "../../include/JumaRE/vertex/VertexBuffer.h"
#include "../../include/JumaRE/vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    RenderEngine::~RenderEngine()
    {
        clearData();
    }

    bool RenderEngine::init(const WindowCreateInfo& mainWindowInfo)
    {
        if (isValid())
        {
            JUTILS_LOG(warning, JSTR("Render engine already initialized"));
            return false;
        }

        WindowController* windowController = createWindowController();
        if (!windowController->initWindowController())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize window controller"));
            delete windowController;
            return false;
        }
        m_WindowController = windowController;
        if (!initInternal(mainWindowInfo))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize render engine"));
            clearInternal();
            return false;
        }
        m_Initialized = true;

        if (!createRenderAssets())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize render assets"));
            clear();
            return false;
        }
        return true;
    }
    bool RenderEngine::initInternal(const WindowCreateInfo& mainWindowInfo)
    {
        return m_WindowController->createMainWindow(mainWindowInfo);
    }

    bool RenderEngine::createRenderAssets()
    {
        if (!m_WindowController->createRenderTargets())
        {
            JUTILS_LOG(error, JSTR("Failed to create render targets for windows"));
            return false;
        }

        RenderPipeline* renderPipeline = createRenderPipelineInternal();
        if (!renderPipeline->init())
        {
            JUTILS_LOG(error, JSTR("Failed to init render pipeline"));
            delete renderPipeline;
            return false;
        }
        m_RenderPipeline = renderPipeline;
        m_WindowController->addPipelineStages();

        constexpr uint8 defaultTextureData[] = { 255, 0, 255, 255 };
        m_DefaultTexture = createTexture({ 1, 1 }, TextureFormat::RGBA8, defaultTextureData);
        if (m_DefaultTexture == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create default texture"));
            return false;
        }

        return true;
    }
    void RenderEngine::clearAssets()
    {
        m_DefaultTexture = nullptr;
        if (m_RenderPipeline != nullptr)
        {
            delete m_RenderPipeline;
            m_RenderPipeline = nullptr;
        }
        if (m_WindowController != nullptr)
        {
            m_WindowController->destroyRenderTargets();
        }
    }

    void RenderEngine::clear()
    {
        if (m_Initialized)
        {
            clearInternal();
            m_Initialized = false;
        }
    }
    void RenderEngine::clearData()
    {
        if (m_WindowController != nullptr)
        {
            delete m_WindowController;
            m_WindowController = nullptr;
        }
        m_RegisteredVertexTypes.clear();
    }

    void RenderEngine::registerObjectInternal(RenderEngineContextObjectBase* object)
    {
        if (object != nullptr)
        {
            object->m_RenderEngine = this;
        }
    }
    RenderPipeline* RenderEngine::createRenderPipelineInternal()
    {
        return createObject<RenderPipeline>();
    }
    
    RenderTarget* RenderEngine::createWindowRenderTarget(const window_id windowID, const TextureSamples samples)
    {
        RenderTarget* renderTarget = allocateRenderTarget();
        if (!renderTarget->init(windowID, samples))
        {
            destroyRenderTarget(renderTarget);
            return nullptr;
        }
        return renderTarget;
    }
    RenderTarget* RenderEngine::createRenderTarget(const TextureFormat format, const math::uvector2& size, const TextureSamples samples)
    {
        RenderTarget* renderTarget = allocateRenderTarget();
        if (!renderTarget->init(format, size, samples))
        {
            destroyRenderTarget(renderTarget);
            return nullptr;
        }
        return renderTarget;
    }
    void RenderEngine::destroyRenderTarget(RenderTarget* renderTarget)
    {
        if (renderTarget != nullptr)
        {
            renderTarget->clearAsset();
            deallocateRenderTarget(renderTarget);
        }
    }

    VertexBuffer* RenderEngine::createVertexBuffer(VertexBufferData* verticesData)
    {
        if (registerVertexType(verticesData) == nullptr)
        {
            return nullptr;
        }

        VertexBuffer* vertexBuffer = allocateVertexBuffer();
        if (!vertexBuffer->init(verticesData))
        {
            destroyVertexBuffer(vertexBuffer);
            return nullptr;
        }
        return vertexBuffer;
    }
    const VertexDescription* RenderEngine::registerVertexType(const VertexBufferData* verticesData)
    {
        if (!isValid() || (verticesData == nullptr))
        {
            return nullptr;
        }

        const jstringID& vertexName = verticesData->getVertexTypeName();
        if (vertexName == jstringID_NONE)
        {
            return nullptr;
        }

        const VertexDescription* description = findVertexType(vertexName);
        if (description != nullptr)
        {
            return description;
        }

        description = &m_RegisteredVertexTypes.add(vertexName, verticesData->getVertexDescription());
        onRegisteredVertexType(vertexName);
        return description;
    }
    void RenderEngine::destroyVertexBuffer(VertexBuffer* vertexBuffer)
    {
        if (vertexBuffer != nullptr)
        {
            vertexBuffer->clearAsset();
            deallocateVertexBuffer(vertexBuffer);
        }
    }

    Shader* RenderEngine::createShader(const jmap<ShaderStageFlags, jstring>& fileNames, jset<jstringID> vertexComponents, 
        jmap<jstringID, ShaderUniform> uniforms)
    {
        Shader* shader = allocateShader();
        if (!shader->init(fileNames, std::move(vertexComponents), std::move(uniforms)))
        {
            destroyShader(shader);
            return nullptr;
        }
        return shader;
    }
    void RenderEngine::destroyShader(Shader* shader)
    {
        if (shader != nullptr)
        {
            shader->clearAsset();
            deallocateShader(shader);
        }
    }

    Material* RenderEngine::createMaterial(Shader* shader)
    {
        Material* material = allocateMaterial();
        if (!material->init(shader))
        {
            destroyMaterial(material);
            return nullptr;
        }
        return material;
    }
    void RenderEngine::destroyMaterial(Material* material)
    {
        if (material != nullptr)
        {
            material->clearAsset();
            deallocateMaterial(material);
        }
    }

    Texture* RenderEngine::createTexture(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        Texture* texture = allocateTexture();
        if (!texture->init(size, format, data))
        {
            destroyTexture(texture);
            return nullptr;
        }
        return texture;
    }
    void RenderEngine::destroyTexture(Texture* texture)
    {
        if (texture != nullptr)
        {
            texture->clearAsset();
            deallocateTexture(texture);
        }
    }
}
