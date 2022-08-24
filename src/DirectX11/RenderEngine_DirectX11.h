// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "../../include/JumaRE/RenderEngine.h"

#include "../../include/JumaRE/texture/TextureSamplerType.h"

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

        virtual math::vector2 getScreenCoordinateModifier() const override { return { 1.0f, -1.0f }; }

    protected:

        virtual bool initInternal(const WindowCreateInfo& mainWindowInfo) override;
        virtual void clearInternal() override;

        virtual WindowController* createWindowController() override;
        virtual VertexBuffer* createVertexBufferInternal() override;
        virtual Texture* createTextureInternal() override;
        virtual Shader* createShaderInternal() override;
        virtual Material* createMaterialInternal() override;
        virtual RenderTarget* createRenderTargetInternal() override;

    private:

        ID3D11Device* m_Device = nullptr;
        ID3D11DeviceContext* m_DeviceContext = nullptr;

        jmap<DepthStencilState, ID3D11DepthStencilState*> m_DepthStencilStates;
        jmap<RasterizationState, ID3D11RasterizerState*> m_RasterizerStates;
        jmap<BlendState, ID3D11BlendState*> m_BlendStates;
        jmap<TextureSamplerType, ID3D11SamplerState*> m_TextureSamplers;


        bool createDirectXDevice();

        void clearDirectX();
    };
}

#endif
