// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL)

#include "RenderEngine_OpenGL.h"

#include <GL/glew.h>

#include "window/WindowControllerImpl_OpenGL.h"

namespace JumaRenderEngine
{
    RenderEngine_OpenGL::~RenderEngine_OpenGL()
    {
        clearOpenGL();
    }

    bool RenderEngine_OpenGL::initAsyncAssetTaskQueueWorker(const int32 workerIndex)
    {
        return getWindowController<WindowController_OpenGL>()->createContextForAsyncAssetTaskQueueWorker(workerIndex);
    }
    bool RenderEngine_OpenGL::initAsyncAssetTaskQueueWorkerThread(const int32 workerIndex)
    {
        return getWindowController<WindowController_OpenGL>()->initAsyncAssetTaskQueueWorkerThread(workerIndex);
    }
    void RenderEngine_OpenGL::clearAsyncAssetTaskQueueWorkerThread(const int32 workerIndex)
    {
        getWindowController<WindowController_OpenGL>()->clearAsyncAssetTaskQueueWorkerThread(workerIndex);
    }
    void RenderEngine_OpenGL::clearAsyncAssetTaskQueueWorker(const int32 workerIndex)
    {
        getWindowController<WindowController_OpenGL>()->destroyContextForAsyncAssetTaskQueueWorker(workerIndex);
    }

    void RenderEngine_OpenGL::clearInternal()
    {
        clearOpenGL();
        Super::clearInternal();
    }
    void RenderEngine_OpenGL::clearOpenGL()
    {
        clearAssets();
        m_MaterialsPool.clear();
        m_TexturesPool.clear();
        m_ShadersPool.clear();
        m_VertexBuffersPool.clear();
        m_RenderTargetsPool.clear();

        for (const auto& sampler : m_SamplerObjectIndices.values())
        {
            glDeleteSamplers(1, &sampler);
        }
        m_SamplerObjectIndices.clear();
    }

    WindowController* RenderEngine_OpenGL::createWindowController()
    {
        return CreateWindowController_OpenGL();
    }

    uint32 RenderEngine_OpenGL::getTextureSamplerIndex(const TextureSamplerType sampler)
    {
        const uint32* samplerIndexPtr = m_SamplerObjectIndices.find(sampler);
        if (samplerIndexPtr != nullptr)
        {
            return *samplerIndexPtr;
        }

        uint32 samplerIndex = 0;
        glGenSamplers(1, &samplerIndex);
        switch (sampler.filterType)
        {
        case TextureFilterType::Point:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            break;
        case TextureFilterType::Bilinear:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            break;
        case TextureFilterType::Trilinear:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            break;
        case TextureFilterType::Anisotropic2:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);
            break;
        case TextureFilterType::Anisotropic4:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
            break;
        case TextureFilterType::Anisotropic8:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
            break;
        case TextureFilterType::Anisotropic16:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
            break;
        default: ;
        }
        switch (sampler.wrapMode)
        {
        case TextureWrapMode::Repeat:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
        case TextureWrapMode::Mirror:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            break;
        case TextureWrapMode::Clamp:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
        default: ;
        }
        return m_SamplerObjectIndices[sampler] = samplerIndex;
    }
}

#endif
