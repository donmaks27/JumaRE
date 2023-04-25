// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12)

#include "Material_DirectX12.h"

#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "RenderTarget_DirectX12.h"
#include "Shader_DirectX12.h"
#include "Texture_DirectX12.h"
#include "VertexBuffer_DirectX12.h"

namespace JumaRenderEngine
{
    Material_DirectX12::~Material_DirectX12()
    {
        clearDirectX();
    }

    bool Material_DirectX12::initInternal()
    {
        const Shader_DirectX12* shader = getShader<Shader_DirectX12>();
        const jmap<jstringID, uint32>& descriptorHeapOffsets = shader->getTextureDescriptorHeapOffsets();

        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12DescriptorHeap* textureDescriptorHeap = nullptr;
        ID3D12DescriptorHeap* samplerDescriptorHeap = nullptr;
        if (!descriptorHeapOffsets.isEmpty())
        {
            textureDescriptorHeap = renderEngine->createDescriptorHeap(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, static_cast<uint32>(descriptorHeapOffsets.getSize()), true
            );
            if (textureDescriptorHeap == nullptr)
            {
                JUTILS_LOG(error, JSTR("Failed to create descriptor heap for material textures"));
                return false;
            }
            samplerDescriptorHeap = renderEngine->createDescriptorHeap(
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, static_cast<uint32>(descriptorHeapOffsets.getSize()), true
            );
            if (samplerDescriptorHeap == nullptr)
            {
                JUTILS_LOG(error, JSTR("Failed to create descriptor heap for material samplers"));
                textureDescriptorHeap->Release();
                return false;
            }
        }

        jmap<uint32, DirectX12Buffer*> buffers;
        for (const auto& [bufferID, bufferDescription] : shader->getUniformBufferDescriptions())
        {
            DirectX12Buffer* newBuffer = buffers.add(bufferID, renderEngine->getBuffer());
            if ((newBuffer == nullptr) || !newBuffer->initAccessedGPU(bufferDescription.size, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
            {
                JUTILS_LOG(error, JSTR("Failed to create uniform buffer"));
                for (const auto& buffer : buffers.values())
                {
                    renderEngine->returnBuffer(buffer);
                }
                if (textureDescriptorHeap != nullptr)
                {
                    textureDescriptorHeap->Release();
                    samplerDescriptorHeap->Release();
                }
                return false;
            }
        }

        m_UniformBuffers = std::move(buffers);
        m_TextureDescriptorHeap = textureDescriptorHeap;
        m_SamplerDescriptorHeap = samplerDescriptorHeap;
        return true;
    }

    void Material_DirectX12::onClearAsset()
    {
        clearDirectX();
        Super::onClearAsset();
    }
    void Material_DirectX12::clearDirectX()
    {
        if (!m_UniformBuffers.isEmpty())
        {
            RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
            for (const auto& buffer : m_UniformBuffers.values())
            {
                renderEngine->returnBuffer(buffer);
            }
            m_UniformBuffers.clear();
        }

        if (m_SamplerDescriptorHeap != nullptr)
        {
            m_SamplerDescriptorHeap->Release();
            m_SamplerDescriptorHeap = nullptr;
        }
        if (m_TextureDescriptorHeap != nullptr)
        {
            m_TextureDescriptorHeap->Release();
            m_TextureDescriptorHeap = nullptr;
        }
    }

    bool Material_DirectX12::bindMaterial(const RenderOptions_DirectX12* renderOptions, VertexBuffer_DirectX12* vertexBuffer)
    {
        if (!updateUniformData())
        {
            return false;
        }

        MaterialProperties properties = getMaterialProperties();
        properties.depthEnabled &= renderOptions->renderTarget->isDepthEnabled() && renderOptions->renderStageProperties.depthEnabled;

        Shader_DirectX12* shader = getShader<Shader_DirectX12>();
        if (!shader->bindShader(renderOptions, vertexBuffer, properties))
        {
            return false;
        }

        ID3D12GraphicsCommandList2* commandList = renderOptions->renderCommandList->get();
        for (const auto& [bufferIndex, bufferLocation] : shader->getUniformBufferParamIndices())
        {
            commandList->SetGraphicsRootConstantBufferView(bufferIndex, m_UniformBuffers[bufferLocation]->get()->GetGPUVirtualAddress());
        }

        if (m_TextureDescriptorHeap != nullptr)
        {
            const uint32 paramIndex = static_cast<uint32>(m_UniformBuffers.getSize());

            ID3D12DescriptorHeap* const descriptorHeaps[2] = { m_TextureDescriptorHeap, m_SamplerDescriptorHeap };
            commandList->SetDescriptorHeaps(2, descriptorHeaps);
            commandList->SetGraphicsRootDescriptorTable(paramIndex, m_TextureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
            commandList->SetGraphicsRootDescriptorTable(paramIndex + 1, m_SamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
        }
        return true;
    }

    bool Material_DirectX12::updateUniformData()
    {
        const jset<jstringID>& notUpdatedParams = getNotUpdatedParams();
        if (notUpdatedParams.isEmpty())
        {
            return true;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12Device2* device = renderEngine->getDevice();
        const Texture_DirectX12* defaultTextureValue = dynamic_cast<const Texture_DirectX12*>(renderEngine->getDefaultTexture());

        const Shader_DirectX12* shader = getShader<Shader_DirectX12>();
        const jmap<jstringID, uint32>& descriptorHeapOffsets = shader->getTextureDescriptorHeapOffsets();
        const MaterialParamsStorage& params = getMaterialParams();
        const jmap<jstringID, ShaderUniform>& uniforms = getShader()->getUniforms();

        for (const auto& paramName : notUpdatedParams)
        {
            const ShaderUniform* uniformPtr = uniforms.find(paramName);
            if (uniformPtr == nullptr)
            {
                continue;
            }
            const ShaderUniform& uniform = *uniformPtr;

            if (uniform.type == ShaderUniformType::Texture)
            {
                ShaderUniformInfo<ShaderUniformType::Texture>::value_type value;
                if (params.getValue<ShaderUniformType::Texture>(paramName, value))
                {
                    const uint32* descriptorHeapIndex = descriptorHeapOffsets.find(paramName);
                    if (descriptorHeapIndex == nullptr)
                    {
                        continue;
                    }

                    const Texture_DirectX12* textureValue = dynamic_cast<Texture_DirectX12*>(value);
                    ID3D12DescriptorHeap* srv = nullptr;
                    if (textureValue != nullptr)
                    {
                        srv = textureValue->getSRV();
                    }
                    else
                    {
                        const RenderTarget_DirectX12* renderTargetValue = dynamic_cast<RenderTarget_DirectX12*>(value);
                        if (renderTargetValue != nullptr)
                        {
                            srv = renderTargetValue->getSRV();
                        }
                        else if (defaultTextureValue != nullptr)
                        {
                            srv = defaultTextureValue->getSRV();
                        }
                        else
                        {
                            throw std::exception("Invalid default texture");
                        }
                    }
                    if (srv == nullptr)
                    {
                        JUTILS_LOG(error, JSTR("Failed to get DX12 texture descriptor"));
                        continue;
                    }

                    const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor = srv->GetCPUDescriptorHandleForHeapStart();
                    const D3D12_CPU_DESCRIPTOR_HANDLE dstDescriptor = renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(
                        m_TextureDescriptorHeap, *descriptorHeapIndex
                    );
                    device->CopyDescriptorsSimple(1, dstDescriptor, srcDescriptor, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                    const D3D12_CPU_DESCRIPTOR_HANDLE srcSamplerDescriptor = renderEngine->getSamplerDescription(value != nullptr ? value->getSamplerType() : TextureSamplerType());
                    const D3D12_CPU_DESCRIPTOR_HANDLE dstSamplerDescriptor = renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>(
                        m_SamplerDescriptorHeap, *descriptorHeapIndex
                    );
                    device->CopyDescriptorsSimple(1, dstSamplerDescriptor, srcSamplerDescriptor, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
                }
            }
            else
            {
                DirectX12Buffer* buffer = m_UniformBuffers[uniform.shaderLocation];
                switch (uniform.type)
                {
                case ShaderUniformType::Float:
                    {
                        ShaderUniformInfo<ShaderUniformType::Float>::value_type value;
                        if (params.getValue<ShaderUniformType::Float>(paramName, value))
                        {
                            buffer->initMappedData();
                            buffer->setMappedData(&value, sizeof(value), uniform.shaderBlockOffset);
                        }
                    }
                    break;
                case ShaderUniformType::Vec2:
                    {
                        ShaderUniformInfo<ShaderUniformType::Vec2>::value_type value;
                        if (params.getValue<ShaderUniformType::Vec2>(paramName, value))
                        {
                            buffer->initMappedData();
                            buffer->setMappedData(&value, sizeof(value), uniform.shaderBlockOffset);
                        }
                    }
                    break;
                case ShaderUniformType::Vec4:
                    {
                        ShaderUniformInfo<ShaderUniformType::Vec4>::value_type value;
                        if (params.getValue<ShaderUniformType::Vec4>(paramName, value))
                        {
                            buffer->initMappedData();
                            buffer->setMappedData(&value, sizeof(value), uniform.shaderBlockOffset);
                        }
                    }
                    break;
                case ShaderUniformType::Mat4:
                    {
                        ShaderUniformInfo<ShaderUniformType::Mat4>::value_type value;
                        if (params.getValue<ShaderUniformType::Mat4>(paramName, value))
                        {
                            buffer->initMappedData();
                            buffer->setMappedData(&value, sizeof(value), uniform.shaderBlockOffset);
                        }
                    }
                    break;

                default: ;
                }
            }
        }
        clearParamsForUpdate();

        DirectX12CommandQueue* commandQueue = renderEngine->getCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
        DirectX12CommandList* commandList = commandQueue->getCommandList();
        for (const auto& buffer : m_UniformBuffers.values())
        {
            buffer->flushMappedData(commandList, false);
        }
        commandList->execute();
        commandList->waitForFinish();
        commandQueue->returnCommandList(commandList);

        clearParamsForUpdate();
        return true;
    }
}

#endif
