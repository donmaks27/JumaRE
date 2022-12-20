// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VertexBuffer_DirectX12.h"
#if defined(JUMARE_ENABLE_DX12)

#include "Shader_DirectX12.h"

#include <d3dcompiler.h>

#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "DirectX12Objects/DirectX12PipelineStateStreamObjects.h"
#include "../DirectX/TextureFormat_DirectX.h"
#include "../../include/JumaRE/RenderTarget.h"

namespace JumaRenderEngine
{
    struct DirectX12_PipelineStateStreamDescription
    {
        DirectX12_PipelineStateStream_ROOT_SIGNATURE rootSignature;
        DirectX12_PipelineStateStream_VS VS;
        DirectX12_PipelineStateStream_GS GS;
        DirectX12_PipelineStateStream_PS PS;

        DirectX12_PipelineStateStream_INPUT_LAYOUT inputLayout;
        DirectX12_PipelineStateStream_RENDER_TARGET_FORMATS RTVFormats;
        DirectX12_PipelineStateStream_DEPTH_STENCIL_FORMAT DSVFormat;

        DirectX12_PipelineStateStream_PRIMITIVE_TOPOLOGY primitiveTopologyType;
        DirectX12_PipelineStateStream_DEPTH_STENCIL1 depthStencil;
        DirectX12_PipelineStateStream_RASTERIZER rasterizer;
        DirectX12_PipelineStateStream_SAMPLE_DESC sampleDescription;
        DirectX12_PipelineStateStream_BLEND blendDescription;
    };

    bool CheckedRootSignatureSupport = false;
    bool SupportedRootSignature_1_1 = false;

    ID3DBlob* LoadDirectX12ShaderFile(const jstring& fileName, const bool optional)
    {
        std::wstring fileNameWide;
        fileNameWide.resize(fileName.getSize() + 1);
        const int32 fileNameWideSize = MultiByteToWideChar(CP_UTF8, 0, *fileName, fileName.getSize(), &fileNameWide[0], static_cast<int>(fileNameWide.size()));
        if (fileNameWideSize <= 0)
        {
            if (!optional)
            {
                JUTILS_LOG(error, JSTR("Failed to convert UTF-8 file name {} to WCHAR"), fileName);
            }
            return nullptr;
        }

        ID3DBlob* shaderBlob = nullptr;
        const HRESULT result = D3DReadFileToBlob(fileNameWide.c_str(), &shaderBlob);
        if (result < 0)
        {
            if (!optional)
            {
                JUTILS_ERROR_LOG(result, JSTR("Failed read shader file {} into DirectX12 blob"), fileName);
            }
            return nullptr;
        }

        return shaderBlob;
    }
    ID3DBlob* LoadDirectX12ShaderFile(const jmap<ShaderStageFlags, jstring>& fileNames, const ShaderStageFlags shaderStage, const bool optional)
    {
        const jstring* fileName = fileNames.find(shaderStage);
        if (fileName == nullptr)
        {
            if (!optional)
            {
                JUTILS_LOG(error, JSTR("Missed file name for required shader stage"));
            }
            return nullptr;
        }
        return LoadDirectX12ShaderFile(*fileName, optional);
    }

    constexpr D3D12_SHADER_VISIBILITY GetDirectX12ShaderParamVisibility(const uint8 stages)
    {
        const bool vertexShaderVisible = (stages & SHADER_STAGE_VERTEX) != 0;
        const bool fragmentShaderVisible = (stages & SHADER_STAGE_FRAGMENT) != 0;
        if (vertexShaderVisible && fragmentShaderVisible)
        {
            return D3D12_SHADER_VISIBILITY_ALL;
        }
        return fragmentShaderVisible ? D3D12_SHADER_VISIBILITY_PIXEL : D3D12_SHADER_VISIBILITY_VERTEX;
    }
    ID3DBlob* CreateDirectX12RootSignatureBlob(const jmap<uint32, ShaderUniformBufferDescription>& uniformBuffers, 
        const jmap<jstringID, ShaderUniform>& uniforms, jmap<uint32, uint32>& outBufferParamIndices, 
        jmap<jstringID, uint32>& outDescriptorHeapOffsets)
    {
        ID3DBlob* resultBlob = nullptr;
        HRESULT result = 0;
        if (SupportedRootSignature_1_1)
        {
            jarray<D3D12_ROOT_PARAMETER1> rootSignatureParams;
            for (const auto& bufferDescription : uniformBuffers)
            {
                outBufferParamIndices.add(rootSignatureParams.getSize(), bufferDescription.key);

                D3D12_ROOT_PARAMETER1& parameter = rootSignatureParams.addDefault();
                parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                parameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(bufferDescription.value.shaderStages);
                parameter.Descriptor.ShaderRegister = bufferDescription.key;
                parameter.Descriptor.RegisterSpace = 0;
                parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
            }

            jarray<D3D12_DESCRIPTOR_RANGE1> textureDescriptorRanges;
            jarray<D3D12_DESCRIPTOR_RANGE1> samplerDescriptorRanges;
            uint8 shaderStages = 0;
            for (const auto& uniform : uniforms)
            {
                if (uniform.value.type != ShaderUniformType::Texture)
                {
                    continue;
                }
                shaderStages |= uniform.value.shaderStages;

                const uint32 offset = outDescriptorHeapOffsets.add(uniform.key, textureDescriptorRanges.getSize());
                D3D12_DESCRIPTOR_RANGE1& textureDescriptorRange = textureDescriptorRanges.addDefault();
                textureDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                textureDescriptorRange.NumDescriptors = 1;
                textureDescriptorRange.BaseShaderRegister = uniform.value.shaderLocation;
                textureDescriptorRange.RegisterSpace = 0;
                textureDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                textureDescriptorRange.OffsetInDescriptorsFromTableStart = offset;
                D3D12_DESCRIPTOR_RANGE1& samplerDescriptorRange = samplerDescriptorRanges.addDefault();
                samplerDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                samplerDescriptorRange.NumDescriptors = 1;
                samplerDescriptorRange.BaseShaderRegister = uniform.value.shaderLocation;
                samplerDescriptorRange.RegisterSpace = 0;
                samplerDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                samplerDescriptorRange.OffsetInDescriptorsFromTableStart = offset;
            }
            if (!textureDescriptorRanges.isEmpty() && (shaderStages != 0))
            {
                D3D12_ROOT_PARAMETER1& textureParameter = rootSignatureParams.addDefault();
                textureParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                textureParameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(shaderStages);
                textureParameter.DescriptorTable.NumDescriptorRanges = textureDescriptorRanges.getSize();
                textureParameter.DescriptorTable.pDescriptorRanges = textureDescriptorRanges.getData();

                D3D12_ROOT_PARAMETER1& samplerParameter = rootSignatureParams.addDefault();
                samplerParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                samplerParameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(shaderStages);
                samplerParameter.DescriptorTable.NumDescriptorRanges = samplerDescriptorRanges.getSize();
                samplerParameter.DescriptorTable.pDescriptorRanges = samplerDescriptorRanges.getData();
            }

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription{};
            rootSignatureDescription.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            rootSignatureDescription.Desc_1_1.NumParameters = rootSignatureParams.getSize();
            rootSignatureDescription.Desc_1_1.pParameters = rootSignatureParams.getData();
            rootSignatureDescription.Desc_1_1.NumStaticSamplers = 0;
            rootSignatureDescription.Desc_1_1.pStaticSamplers = nullptr;
            rootSignatureDescription.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &resultBlob, nullptr);
        }
        else
        {
            jarray<D3D12_ROOT_PARAMETER> rootSignatureParams;
            for (const auto& bufferDescription : uniformBuffers)
            {
                outBufferParamIndices.add(rootSignatureParams.getSize(), bufferDescription.key);

                D3D12_ROOT_PARAMETER& parameter = rootSignatureParams.addDefault();
                parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                parameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(bufferDescription.value.shaderStages);
                parameter.Descriptor.ShaderRegister = bufferDescription.key;
                parameter.Descriptor.RegisterSpace = 0;
            }

            jarray<D3D12_DESCRIPTOR_RANGE> textureDescriptorRanges;
            jarray<D3D12_DESCRIPTOR_RANGE> samplerDescriptorRanges;
            uint8 shaderStages = 0;
            for (const auto& uniform : uniforms)
            {
                if (uniform.value.type != ShaderUniformType::Texture)
                {
                    continue;
                }
                shaderStages |= uniform.value.shaderStages;

                const uint32 offset = outDescriptorHeapOffsets.add(uniform.key, textureDescriptorRanges.getSize());
                D3D12_DESCRIPTOR_RANGE& textureDescriptorRange = textureDescriptorRanges.addDefault();
                textureDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                textureDescriptorRange.NumDescriptors = 1;
                textureDescriptorRange.BaseShaderRegister = uniform.value.shaderLocation;
                textureDescriptorRange.RegisterSpace = 0;
                textureDescriptorRange.OffsetInDescriptorsFromTableStart = offset;
                D3D12_DESCRIPTOR_RANGE& samplerDescriptorRange = samplerDescriptorRanges.addDefault();
                samplerDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                samplerDescriptorRange.NumDescriptors = 1;
                samplerDescriptorRange.BaseShaderRegister = uniform.value.shaderLocation;
                samplerDescriptorRange.RegisterSpace = 0;
                samplerDescriptorRange.OffsetInDescriptorsFromTableStart = offset;
            }
            if (!textureDescriptorRanges.isEmpty() && (shaderStages != 0))
            {
                D3D12_ROOT_PARAMETER& textureParameter = rootSignatureParams.addDefault();
                textureParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                textureParameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(shaderStages);
                textureParameter.DescriptorTable.NumDescriptorRanges = textureDescriptorRanges.getSize();
                textureParameter.DescriptorTable.pDescriptorRanges = textureDescriptorRanges.getData();

                D3D12_ROOT_PARAMETER& samplerParameter = rootSignatureParams.addDefault();
                samplerParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                samplerParameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(shaderStages);
                samplerParameter.DescriptorTable.NumDescriptorRanges = samplerDescriptorRanges.getSize();
                samplerParameter.DescriptorTable.pDescriptorRanges = samplerDescriptorRanges.getData();
            }

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription{};
            rootSignatureDescription.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
            rootSignatureDescription.Desc_1_0.NumParameters = rootSignatureParams.getSize();
            rootSignatureDescription.Desc_1_0.pParameters = rootSignatureParams.getData();
            rootSignatureDescription.Desc_1_0.NumStaticSamplers = 0;
            rootSignatureDescription.Desc_1_0.pStaticSamplers = nullptr;
            rootSignatureDescription.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &resultBlob, nullptr);
        }
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to serialize root signature"));
            return nullptr;
        }
        return resultBlob;
    }

    Shader_DirectX12::~Shader_DirectX12()
    {
        clearDirectX();
    }

    bool Shader_DirectX12::initInternal(const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        ID3DBlob* vertexShaderBlob = LoadDirectX12ShaderFile(fileNames, SHADER_STAGE_VERTEX, false);
        if (vertexShaderBlob == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to load DirectX12 vertex shader"));
            return false;
        }
        ID3DBlob* fragmentShaderBlob = LoadDirectX12ShaderFile(fileNames, SHADER_STAGE_FRAGMENT, false);
        if (fragmentShaderBlob == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to load DirectX12 fragment shader"));
            vertexShaderBlob->Release();
            return false;
        }
        
        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12Device2* device = renderEngine->getDevice();
        if (!CheckedRootSignatureSupport)
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            SupportedRootSignature_1_1 = SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));
            CheckedRootSignatureSupport = true;
        }

        jmap<uint32, uint32> bufferParamIndices;
        jmap<jstringID, uint32> descriptorHeapOffsets;
        ID3DBlob* rootSignatureBlob = CreateDirectX12RootSignatureBlob(getUniformBufferDescriptions(), getUniforms(), bufferParamIndices, descriptorHeapOffsets);
        if (rootSignatureBlob == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create DirectX12 root signature blob"));
            fragmentShaderBlob->Release();
            vertexShaderBlob->Release();
            return false;
        }

        ID3D12RootSignature* rootSignature = nullptr;
        const HRESULT result = device->CreateRootSignature(
            0, rootSignatureBlob->GetBufferPointer(), 
            rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)
        );
        rootSignatureBlob->Release();
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX12 root signature"));
            fragmentShaderBlob->Release();
            vertexShaderBlob->Release();
            return false;
        }

        m_RootSignature = rootSignature;
        m_ShaderBytecodes = { { SHADER_STAGE_VERTEX, vertexShaderBlob }, { SHADER_STAGE_FRAGMENT, fragmentShaderBlob } };
        m_UniformBufferParamIndices = std::move(bufferParamIndices);
        m_TextureDescriptorHeapOffsets = std::move(descriptorHeapOffsets);
        return true;
    }

    void Shader_DirectX12::clearAsset()
    {
        clearDirectX();
        Super::clearAsset();
    }
    void Shader_DirectX12::clearDirectX()
    {
        for (const auto& pipelineState : m_PipelineStates)
        {
            pipelineState.value->Release();
        }
        m_PipelineStates.clear();

        m_TextureDescriptorHeapOffsets.clear();
        m_UniformBufferParamIndices.clear();

        for (const auto& bytecode : m_ShaderBytecodes)
        {
            bytecode.value->Release();
        }
        m_ShaderBytecodes.clear();

        if (m_RootSignature != nullptr)
        {
            m_RootSignature->Release();
            m_RootSignature = nullptr;
        }
    }

    bool Shader_DirectX12::bindShader(const RenderOptions_DirectX12* renderOptions, VertexBuffer_DirectX12* vertexBuffer, 
        const MaterialProperties& materialProperties)
    {
        const RenderTarget* renderTarget = renderOptions->renderTarget;
        const TextureFormat colorFormat = renderTarget->getColorFormat();
        const TextureFormat depthFormat = TextureFormat::DEPTH24_STENCIL8;
        const TextureSamples samples = renderTarget->getSampleCount();
        ID3D12PipelineState* pipelineState = getPipelineState({ 
            vertexBuffer->getVertexID(), colorFormat, depthFormat, samples, materialProperties
        });
        if (pipelineState == nullptr)
        {
            return false;
        }

        ID3D12GraphicsCommandList2* commandList = renderOptions->renderCommandList->get();
        commandList->SetGraphicsRootSignature(m_RootSignature);
        commandList->SetPipelineState(pipelineState);
        return true;
    }
    ID3D12PipelineState* Shader_DirectX12::getPipelineState(const PipelineStateID& pipelineStateID)
    {
        ID3D12PipelineState* const* existingPipelineState = m_PipelineStates.find(pipelineStateID);
        if (existingPipelineState != nullptr)
        {
            return *existingPipelineState;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        const RegisteredVertexDescription* vertexDescription = renderEngine->findVertex(pipelineStateID.vertexID);
        if (vertexDescription == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to get description for vertex {}"), pipelineStateID.vertexID);
            return nullptr;
        }
        const jset<jstringID>& requiredComponents = getRequiredVertexComponents();

        uint32 componentOffset = 0;
        jarray<D3D12_INPUT_ELEMENT_DESC> inputLayouts;
        inputLayouts.reserve(getRequiredVertexComponents().getSize());
        for (const auto& componentID : vertexDescription->description.components)
        {
            const VertexComponentDescription* componentDescription = renderEngine->findVertexComponent(componentID);
            if (requiredComponents.contains(componentID))
            {
                DXGI_FORMAT componentFormat = DXGI_FORMAT_UNKNOWN;
                switch (componentDescription->type)
                {
                case VertexComponentType::Float: componentFormat = DXGI_FORMAT_R32_FLOAT; break;
                case VertexComponentType::Vec2:  componentFormat = DXGI_FORMAT_R32G32_FLOAT; break;
                case VertexComponentType::Vec3:  componentFormat = DXGI_FORMAT_R32G32B32_FLOAT; break;
                case VertexComponentType::Vec4:  componentFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
                default: 
                    JUTILS_LOG(error, JSTR("Unsupported type of vertex component {} in vertex {}"), componentDescription->shaderLocation, pipelineStateID.vertexID);
                    return nullptr;
                }
                inputLayouts.add({ 
                    "TEXCOORD", componentDescription->shaderLocation, componentFormat, 0,
                    componentOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
                });
            }
            componentOffset += GetVertexComponentSize(componentDescription->type);
        }
        if (inputLayouts.getSize() != requiredComponents.getSize())
        {
            JUTILS_LOG(error, JSTR("Can't find some of the vertex components in vertex {}"), pipelineStateID.vertexID);
            return nullptr;
        }

        ID3D12Device2* device = renderEngine->getDevice();
        ID3DBlob* vertexShaderBytecode = m_ShaderBytecodes.get(SHADER_STAGE_VERTEX);
        ID3DBlob* fragmentShaderBytecode = m_ShaderBytecodes.get(SHADER_STAGE_FRAGMENT);

        DirectX12_PipelineStateStreamDescription pipelineStateStream;
        pipelineStateStream.rootSignature = m_RootSignature;
        // Shaders
        pipelineStateStream.VS.data.BytecodeLength = vertexShaderBytecode->GetBufferSize();
        pipelineStateStream.VS.data.pShaderBytecode = vertexShaderBytecode->GetBufferPointer();
        pipelineStateStream.GS.data.BytecodeLength = 0;
        pipelineStateStream.GS.data.pShaderBytecode = nullptr;
        pipelineStateStream.PS.data.BytecodeLength = fragmentShaderBytecode->GetBufferSize();
        pipelineStateStream.PS.data.pShaderBytecode = fragmentShaderBytecode->GetBufferPointer();

        // Vertex data
        pipelineStateStream.inputLayout.data.NumElements = inputLayouts.getSize();
        pipelineStateStream.inputLayout.data.pInputElementDescs = inputLayouts.getData();
        // Image format data
        pipelineStateStream.RTVFormats.data.NumRenderTargets = 1;
        pipelineStateStream.RTVFormats.data.RTFormats[0] = GetDirectXFormatByTextureFormat(pipelineStateID.colorFormat);
        pipelineStateStream.DSVFormat = pipelineStateID.properties.depthEnabled || pipelineStateID.properties.stencilEnabled ? GetDirectXFormatByTextureFormat(pipelineStateID.depthFormat) : DXGI_FORMAT_UNKNOWN;

        pipelineStateStream.primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        // Depth data
        pipelineStateStream.depthStencil.data.DepthEnable = pipelineStateID.properties.depthEnabled ? TRUE : FALSE;
        pipelineStateStream.depthStencil.data.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        pipelineStateStream.depthStencil.data.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        // Stencil data
        pipelineStateStream.depthStencil.data.StencilEnable = pipelineStateID.properties.stencilEnabled ? TRUE : FALSE;
        pipelineStateStream.depthStencil.data.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        pipelineStateStream.depthStencil.data.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        pipelineStateStream.depthStencil.data.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        pipelineStateStream.depthStencil.data.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        pipelineStateStream.depthStencil.data.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        pipelineStateStream.depthStencil.data.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        pipelineStateStream.depthStencil.data.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        pipelineStateStream.depthStencil.data.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        pipelineStateStream.depthStencil.data.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        pipelineStateStream.depthStencil.data.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        pipelineStateStream.depthStencil.data.DepthBoundsTestEnable = FALSE;
        // Rasterizer data
        pipelineStateStream.rasterizer.data.FillMode = pipelineStateID.properties.wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
        pipelineStateStream.rasterizer.data.CullMode = pipelineStateID.properties.cullBackFaces ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_FRONT;
        pipelineStateStream.rasterizer.data.FrontCounterClockwise = TRUE;
        pipelineStateStream.rasterizer.data.DepthBias = 0;
        pipelineStateStream.rasterizer.data.DepthBiasClamp = 0.0f;
        pipelineStateStream.rasterizer.data.SlopeScaledDepthBias = 0.0f;
        pipelineStateStream.rasterizer.data.DepthClipEnable = TRUE;
        pipelineStateStream.rasterizer.data.MultisampleEnable = TRUE;
        pipelineStateStream.rasterizer.data.AntialiasedLineEnable = FALSE;
        pipelineStateStream.rasterizer.data.ForcedSampleCount = 0;
        pipelineStateStream.rasterizer.data.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        // Samples
        pipelineStateStream.sampleDescription.data.Count = GetTextureSamplesNumber(pipelineStateID.samplesCount);
        pipelineStateStream.sampleDescription.data.Quality = 0;
        // Blend
        pipelineStateStream.blendDescription.data.AlphaToCoverageEnable = FALSE;
        pipelineStateStream.blendDescription.data.IndependentBlendEnable = FALSE;
        pipelineStateStream.blendDescription.data.RenderTarget[0].BlendEnable = pipelineStateID.properties.blendEnabled ? TRUE : FALSE;
        pipelineStateStream.blendDescription.data.RenderTarget[0].LogicOpEnable = FALSE;
        pipelineStateStream.blendDescription.data.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        pipelineStateStream.blendDescription.data.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        pipelineStateStream.blendDescription.data.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        pipelineStateStream.blendDescription.data.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        pipelineStateStream.blendDescription.data.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        pipelineStateStream.blendDescription.data.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        pipelineStateStream.blendDescription.data.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
        pipelineStateStream.blendDescription.data.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDescription{};
        pipelineStateStreamDescription.SizeInBytes = sizeof(pipelineStateStream);
        pipelineStateStreamDescription.pPipelineStateSubobjectStream = &pipelineStateStream;
        ID3D12PipelineState* pipelineState = nullptr;
        const HRESULT result = device->CreatePipelineState(&pipelineStateStreamDescription, IID_PPV_ARGS(&pipelineState));
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX12 pipeline state"));
            return nullptr;
        }
        return m_PipelineStates[pipelineStateID] = pipelineState;
    }
}

#endif
