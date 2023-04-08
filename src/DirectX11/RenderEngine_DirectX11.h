// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "JumaRE/RenderEngine.h"

#include <jutils/jpool_simple.h>

#include "Material_DirectX11.h"
#include "RenderTarget_DirectX11.h"
#include "Shader_DirectX11.h"
#include "Texture_DirectX11.h"
#include "VertexBuffer_DirectX11.h"

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
        virtual RenderTarget* allocateRenderTarget() override { return m_RenderTargetsPool.getPoolObject(); }
        virtual VertexBuffer* allocateVertexBuffer() override { return m_VertexBuffersPool.getPoolObject(); }
        virtual Shader* allocateShader() override { return m_ShadersPool.getPoolObject(); }
        virtual Material* allocateMaterial() override { return m_MaterialsPool.getPoolObject(); }
        virtual Texture* allocateTexture() override { return m_TexturesPool.getPoolObject(); }

        virtual void deallocateRenderTarget(RenderTarget* renderTarget) override { m_RenderTargetsPool.returnPoolObject(dynamic_cast<RenderTarget_DirectX11*>(renderTarget)); }
        virtual void deallocateVertexBuffer(VertexBuffer* vertexBuffer) override { m_VertexBuffersPool.returnPoolObject(dynamic_cast<VertexBuffer_DirectX11*>(vertexBuffer)); }
        virtual void deallocateShader(Shader* shader) override { m_ShadersPool.returnPoolObject(dynamic_cast<Shader_DirectX11*>(shader)); }
        virtual void deallocateMaterial(Material* material) override { m_MaterialsPool.returnPoolObject(dynamic_cast<Material_DirectX11*>(material)); }
        virtual void deallocateTexture(Texture* texture) override { m_TexturesPool.returnPoolObject(dynamic_cast<Texture_DirectX11*>(texture)); }

    private:

        ID3D11Device* m_Device = nullptr;
        ID3D11DeviceContext* m_DeviceContext = nullptr;
        
        jpool_simple<RenderTarget_DirectX11> m_RenderTargetsPool;
        jpool_simple<VertexBuffer_DirectX11> m_VertexBuffersPool;
        jpool_simple<Shader_DirectX11> m_ShadersPool;
        jpool_simple<Material_DirectX11> m_MaterialsPool;
        jpool_simple<Texture_DirectX11> m_TexturesPool;

        jmap<DepthStencilState, ID3D11DepthStencilState*> m_DepthStencilStates;
        jmap<RasterizationState, ID3D11RasterizerState*> m_RasterizerStates;
        jmap<BlendState, ID3D11BlendState*> m_BlendStates;
        jmap<TextureSamplerType, ID3D11SamplerState*> m_TextureSamplers;


        bool createDirectXDevice();

        void clearDirectX();
    };
}

#endif
