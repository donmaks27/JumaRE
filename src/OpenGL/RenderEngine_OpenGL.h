// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "JumaRE/RenderEngine.h"

#include <jutils/jpool_simple.h>

#include "Material_OpenGL.h"
#include "RenderTarget_OpenGL.h"
#include "Shader_OpenGL.h"
#include "Texture_OpenGL.h"
#include "VertexBuffer_OpenGL.h"

namespace JumaRenderEngine
{
    class RenderEngine_OpenGL final : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_OpenGL() = default;
        virtual ~RenderEngine_OpenGL() override;

        virtual RenderAPI getRenderAPI() const override { return RenderAPI::OpenGL; }

        uint32 getTextureSamplerIndex(TextureSamplerType sampler);

    protected:

        virtual bool initAsyncAssetTaskQueueWorker(int32 workerIndex) override;
        virtual bool initAsyncAssetTaskQueueWorkerThread(int32 workerIndex) override;
        virtual void clearAsyncAssetTaskQueueWorkerThread(int32 workerIndex) override;
        virtual void clearAsyncAssetTaskQueueWorker(int32 workerIndex) override;
        virtual void clearInternal() override;

        virtual WindowController* createWindowController() override;
        virtual RenderTarget* allocateRenderTarget() override { return m_RenderTargetsPool.getPoolObject(); }
        virtual VertexBuffer* allocateVertexBuffer() override { return m_VertexBuffersPool.getPoolObject(); }
        virtual Shader* allocateShader() override { return m_ShadersPool.getPoolObject(); }
        virtual Material* allocateMaterial() override { return m_MaterialsPool.getPoolObject(); }
        virtual Texture* allocateTexture() override { return m_TexturesPool.getPoolObject(); }

        virtual void deallocateRenderTarget(RenderTarget* renderTarget) override { m_RenderTargetsPool.returnPoolObject(dynamic_cast<RenderTarget_OpenGL*>(renderTarget)); }
        virtual void deallocateVertexBuffer(VertexBuffer* vertexBuffer) override { m_VertexBuffersPool.returnPoolObject(dynamic_cast<VertexBuffer_OpenGL*>(vertexBuffer)); }
        virtual void deallocateShader(Shader* shader) override { m_ShadersPool.returnPoolObject(dynamic_cast<Shader_OpenGL*>(shader)); }
        virtual void deallocateMaterial(Material* material) override { m_MaterialsPool.returnPoolObject(dynamic_cast<Material_OpenGL*>(material)); }
        virtual void deallocateTexture(Texture* texture) override { m_TexturesPool.returnPoolObject(dynamic_cast<Texture_OpenGL*>(texture)); }

    private:

        jmap<TextureSamplerType, uint32> m_SamplerObjectIndices;
        
        jpool_simple<RenderTarget_OpenGL> m_RenderTargetsPool;
        jpool_simple<VertexBuffer_OpenGL> m_VertexBuffersPool;
        jpool_simple<Shader_OpenGL> m_ShadersPool;
        jpool_simple<Material_OpenGL> m_MaterialsPool;
        jpool_simple<Texture_OpenGL> m_TexturesPool;


        void clearOpenGL();
    };
}

#endif
