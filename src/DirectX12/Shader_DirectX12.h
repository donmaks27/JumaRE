// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "../../include/JumaRE/material/Shader.h"

#include <d3dcommon.h>

#include "../../include/JumaRE/material/MaterialProperties.h"
#include "../../include/JumaRE/texture/TextureFormat.h"
#include "../../include/JumaRE/texture/TextureSamples.h"

struct ID3D12PipelineState;
struct ID3D12RootSignature;

namespace JumaRenderEngine
{
    class VertexBuffer_DirectX12;
    struct RenderOptions_DirectX12;

    class Shader_DirectX12 final : public Shader
    {
        using Super = Shader;

    public:
        Shader_DirectX12() = default;
        virtual ~Shader_DirectX12() override;

        const jmap<uint32, uint32>& getUniformBufferParamIndices() const { return m_UniformBufferParamIndices; }
        const jmap<jstringID, uint32>& getTextureDescriptorHeapOffsets() const { return m_TextureDescriptorHeapOffsets; }

        bool bindShader(const RenderOptions_DirectX12* renderOptions, VertexBuffer_DirectX12* vertexBuffer, 
            const MaterialProperties& materialProperties);

    protected:

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) override;
        virtual void clearAsset() override;

    private:
        
        struct PipelineStateID
        {
            vertex_id vertexID = vertex_id_NONE;
            TextureFormat colorFormat = TextureFormat::RGBA8;
            TextureFormat depthFormat = TextureFormat::DEPTH24_STENCIL8;
            TextureSamples samplesCount = TextureSamples::X1;

            MaterialProperties properties;

            inline bool operator<(const PipelineStateID& otherID) const;
        };

        ID3D12RootSignature* m_RootSignature = nullptr;
        jmap<PipelineStateID, ID3D12PipelineState*> m_PipelineStates;

        jmap<ShaderStageFlags, ID3DBlob*> m_ShaderBytecodes;
        jmap<uint32, uint32> m_UniformBufferParamIndices;
        jmap<jstringID, uint32> m_TextureDescriptorHeapOffsets;


        void clearDirectX();

        ID3D12PipelineState* getPipelineState(const PipelineStateID& pipelineStateID);
    };

    bool Shader_DirectX12::PipelineStateID::operator<(const PipelineStateID& otherID) const
    {
        if (vertexID != otherID.vertexID)
        {
            return vertexID < otherID.vertexID;
        }
        if (colorFormat != otherID.colorFormat)
        {
            return colorFormat < otherID.colorFormat;
        }
        if (depthFormat != otherID.depthFormat)
        {
            return depthFormat < otherID.depthFormat;
        }
        if (samplesCount != otherID.samplesCount)
        {
            return samplesCount < otherID.samplesCount;
        }
        return properties < otherID.properties;
    }
}

#endif
