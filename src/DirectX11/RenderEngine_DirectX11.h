// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "JumaRE/RenderEngine.h"

#include "Material_DirectX11.h"
#include "RenderTarget_DirectX11.h"
#include "Shader_DirectX11.h"
#include "Texture_DirectX11.h"
#include "VertexBuffer_DirectX11.h"
#include "../utils/RenderEngineObjectsPool.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11SamplerState;

namespace JumaRenderEngine
{
    class RenderEngine_DirectX11 final : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_DirectX11() = default;
        virtual ~RenderEngine_DirectX11() override;

        struct DepthStencilState
        {
            bool depthEnabled = true;
            bool stencilEnabled = false;

            constexpr bool operator<(const DepthStencilState& description) const
            {
                return depthEnabled != description.depthEnabled ? !depthEnabled : (stencilEnabled < description.stencilEnabled);
            }
        };
        struct RasterizationState
        {
            bool cullBackFaces = true;
            bool wireframe = false;

            constexpr bool operator<(const RasterizationState& description) const
            {
                return cullBackFaces != description.cullBackFaces ? !cullBackFaces : (wireframe < description.wireframe);
            }
        };
        struct BlendState
        {
            bool blendEnabled = true;

            constexpr bool operator<(const BlendState& description) const { return blendEnabled < description.blendEnabled; }
        };


        virtual RenderAPI getRenderAPI() const override { return RenderAPI::DirectX11; }

        ID3D11Device* getDevice() const { return m_Device; }
        ID3D11DeviceContext* getDeviceContext() const { return m_DeviceContext; }

        ID3D11DepthStencilState* getDepthStencilState(const DepthStencilState& description);
        ID3D11RasterizerState* getRasterizerState(const RasterizationState& description);
        ID3D11BlendState* getBlendState(const BlendState& description);
        ID3D11SamplerState* getTextureSampler(TextureSamplerType samplerType);

    protected:

        virtual bool initInternal(const WindowCreateInfo& mainWindowInfo) override;
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

        ID3D11Device* m_Device = nullptr;
        ID3D11DeviceContext* m_DeviceContext = nullptr;
        
        RenderEngineObjectsPool<RenderTarget, RenderTarget_DirectX11> m_RenderTargetsPool;
        RenderEngineObjectsPool<VertexBuffer, VertexBuffer_DirectX11> m_VertexBuffersPool;
        RenderEngineObjectsPool<Shader, Shader_DirectX11> m_ShadersPool;
        RenderEngineObjectsPool<Material, Material_DirectX11> m_MaterialsPool;
        RenderEngineObjectsPool<Texture, Texture_DirectX11> m_TexturesPool;

        jmap<DepthStencilState, ID3D11DepthStencilState*> m_DepthStencilStates;
        jmap<RasterizationState, ID3D11RasterizerState*> m_RasterizerStates;
        jmap<BlendState, ID3D11BlendState*> m_BlendStates;
        jmap<TextureSamplerType, ID3D11SamplerState*> m_TextureSamplers;


        bool createDirectXDevice();

        void clearDirectX();
    };
}

#endif
