// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "../../include/JumaRE/RenderEngine.h"

#include "Material_OpenGL.h"
#include "RenderTarget_OpenGL.h"
#include "Shader_OpenGL.h"
#include "Texture_OpenGL.h"
#include "VertexBuffer_OpenGL.h"
#include "../utils/RenderEngineObjectsPool.h"
#include "../../include/JumaRE/texture/TextureSamplerType.h"

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

        virtual math::vector2 getScreenCoordinateModifier() const override { return { 1.0f, -1.0f }; }
        virtual bool shouldFlipLoadedTextures() const override { return true; }

    protected:

        virtual void clearInternal() override;

        virtual WindowController* createWindowController() override;
        virtual RenderTarget* allocateRenderTarget() override { return m_RenderTargetsPool.getObject(this); }
        virtual VertexBuffer* allocateVertexBuffer() override { return m_VertexBuffersPool.getObject(this); }
        virtual Shader* allocateShader() override { return m_ShadersPool.getObject(this); }
        virtual Material* allocateMaterial() override { return m_MaterialsPool.getObject(this); }
        virtual Texture* allocateTexture() override { return m_TexturesPool.getObject(this); }

        virtual void deallocateRenderTarget(RenderTarget* renderTarget) override { m_RenderTargetsPool.returnObject(renderTarget); }
        virtual void deallocateVertexBuffer(VertexBuffer* vertexBuffer) override { m_VertexBuffersPool.returnObject(vertexBuffer); }
        virtual void deallocateShader(Shader* shader) override { m_ShadersPool.returnObject(shader); }
        virtual void deallocateMaterial(Material* material) override { m_MaterialsPool.returnObject(material); }
        virtual void deallocateTexture(Texture* texture) override { m_TexturesPool.returnObject(texture); }

    private:

        jmap<TextureSamplerType, uint32> m_SamplerObjectIndices;
        
        RenderEngineObjectsPool<RenderTarget, RenderTarget_OpenGL> m_RenderTargetsPool;
        RenderEngineObjectsPool<VertexBuffer, VertexBuffer_OpenGL> m_VertexBuffersPool;
        RenderEngineObjectsPool<Shader, Shader_OpenGL> m_ShadersPool;
        RenderEngineObjectsPool<Material, Material_OpenGL> m_MaterialsPool;
        RenderEngineObjectsPool<Texture, Texture_OpenGL> m_TexturesPool;


        void clearOpenGL();
    };
}

#endif
