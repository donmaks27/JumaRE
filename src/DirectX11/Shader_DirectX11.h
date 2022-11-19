// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "../../include/JumaRE/material/Shader.h"

#include <d3dcompiler.h>

#include "../../include/JumaRE/vertex/VertexDescription.h"

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

namespace JumaRenderEngine
{
    class VertexBuffer_DirectX11;
    struct RenderOptions;

    class Shader_DirectX11 final : public Shader
    {
        using Super = Shader;

    public:
        Shader_DirectX11() = default;
        virtual ~Shader_DirectX11() override;

        bool bindShader(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer);
        void unbindShader(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer);

    protected:

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) override;
        virtual void clearAsset() override;

    private:

        ID3DBlob* m_VertexShaderBlob = nullptr;
        ID3D11VertexShader* m_VertexShader = nullptr;
        ID3D11PixelShader* m_FragmentShader = nullptr;

        jmap<vertex_id, ID3D11InputLayout*> m_VertexInputLayouts;


        void clearDirectX();
        
        ID3D11InputLayout* getVertexInputLayout(vertex_id vertexID);
    };
}

#endif
