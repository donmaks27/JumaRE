﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX11)

#include "Material_DirectX11.h"

#include <d3d11.h>

#include "RenderEngine_DirectX11.h"
#include "RenderTarget_DirectX11.h"
#include "Shader_DirectX11.h"
#include "Texture_DirectX11.h"
#include "JumaRE/RenderOptions.h"

namespace JumaRenderEngine
{
    Material_DirectX11::~Material_DirectX11()
    {
        clearDirectX();
    }

    bool Material_DirectX11::initInternal()
    {
        ID3D11Device* device = getRenderEngine<RenderEngine_DirectX11>()->getDevice();

        const jmap<uint32, ShaderUniformBufferDescription>& uniformBufferDescriptions = getShader()->getUniformBufferDescriptions();
        if (!uniformBufferDescriptions.isEmpty())
        {
            for (const auto& [bufferID, bufferDescription] : uniformBufferDescriptions)
            {
                static constexpr uint32 mask = 15;
                const uint32 size = (bufferDescription.size + mask) & ~mask;
                D3D11_BUFFER_DESC description{};
                description.ByteWidth = size;
                description.Usage = D3D11_USAGE_DYNAMIC;
                description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                description.MiscFlags = 0;
                description.StructureByteStride = description.ByteWidth;
                ID3D11Buffer* uniformBuffer = nullptr;
                const HRESULT result = device->CreateBuffer(&description, nullptr, &uniformBuffer);
                if (FAILED(result))
                {
                    JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX11 uniform buffer"));
                    continue;
                }

                m_UniformBuffers.add(bufferID, { uniformBuffer, bufferDescription.shaderStages });
            }
        }
        return true;
    }

    void Material_DirectX11::onClearAsset()
    {
        clearDirectX();
        Super::onClearAsset();
    }
    void Material_DirectX11::clearDirectX()
    {
        for (const auto& uniformBuffer : m_UniformBuffers.values())
        {
            uniformBuffer.buffer->Release();
        }
        m_UniformBuffers.clear();
    }

    bool Material_DirectX11::bindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer)
    {
        if (!getShader<Shader_DirectX11>()->bindShader(renderOptions, vertexBuffer))
        {
            return false;
        }

        RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        ID3D11DeviceContext* deviceContext = renderEngine->getDeviceContext();
        
        bindUniforms(deviceContext);

        MaterialProperties properties = getMaterialProperties();
        properties.depthEnabled &= renderOptions->renderStageProperties.depthEnabled;

        ID3D11RasterizerState* rasterizerState = renderEngine->getRasterizerState({ properties.cullBackFaces, properties.wireframe });
        if (rasterizerState != nullptr)
        {
            deviceContext->RSSetState(rasterizerState);
        }
        ID3D11DepthStencilState* depthStencilState = renderEngine->getDepthStencilState({ properties.depthEnabled, properties.stencilEnabled });
        if (depthStencilState != nullptr)
        {
            deviceContext->OMSetDepthStencilState(depthStencilState, 1);
        }
        ID3D11BlendState* blendState = renderEngine->getBlendState({ properties.blendEnabled });
        if (blendState != nullptr)
        {
            deviceContext->OMSetBlendState(blendState, nullptr, D3D11_DEFAULT_SAMPLE_MASK);
        }
        return true;
    }
    void Material_DirectX11::unbindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer)
    {
        ID3D11DeviceContext* deviceContext = getRenderEngine<RenderEngine_DirectX11>()->getDeviceContext();
        ID3D11ShaderResourceView* emptyTextureView = nullptr;
        for (const auto& uniform : getShader()->getUniforms().values())
        {
            if (uniform.type != ShaderUniformType::Texture)
            {
                continue;
            }
            if (uniform.shaderStages & SHADER_STAGE_VERTEX)
            {
                deviceContext->VSSetShaderResources(uniform.shaderLocation, 1, &emptyTextureView);
            }
            if (uniform.shaderStages & SHADER_STAGE_FRAGMENT)
            {
                deviceContext->PSSetShaderResources(uniform.shaderLocation, 1, &emptyTextureView);
            }
        }
        ID3D11Buffer* emptyBuffer = nullptr;
        for (const auto& [bufferID, uniformBuffer] : m_UniformBuffers)
        {
            if (uniformBuffer.shaderStages & SHADER_STAGE_VERTEX)
            {
                deviceContext->VSSetConstantBuffers(bufferID, 1, &emptyBuffer);
            }
            if (uniformBuffer.shaderStages & SHADER_STAGE_FRAGMENT)
            {
                deviceContext->PSSetConstantBuffers(bufferID, 1, &emptyBuffer);
            }
        }

        getShader<Shader_DirectX11>()->unbindShader(renderOptions, vertexBuffer);
    }
    
    void Material_DirectX11::bindUniforms(ID3D11DeviceContext* deviceContext)
    {
        updateUniformBuffersData(deviceContext);

        for (const auto& [bufferID, uniformBuffer] : m_UniformBuffers)
        {
            if (uniformBuffer.shaderStages & SHADER_STAGE_VERTEX)
            {
                deviceContext->VSSetConstantBuffers(bufferID, 1, &uniformBuffer.buffer);
            }
            if (uniformBuffer.shaderStages & SHADER_STAGE_FRAGMENT)
            {
                deviceContext->PSSetConstantBuffers(bufferID, 1, &uniformBuffer.buffer);
            }
        }
        
        RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        const Texture_DirectX11* defaultTexture = dynamic_cast<const Texture_DirectX11*>(renderEngine->getDefaultTexture());
        const MaterialParamsStorage& materialParams = getMaterialParams();
        for (const auto& [uniformID, uniform] : getShader()->getUniforms())
        {
            if (uniform.type != ShaderUniformType::Texture)
            {
                continue;
            }
            ShaderUniformInfo<ShaderUniformType::Texture>::value_type value;
            if (!materialParams.getValue<ShaderUniformType::Texture>(uniformID, value))
            {
                continue;
            }
            ID3D11SamplerState* sampler = renderEngine->getTextureSampler(value != nullptr ? value->getSamplerType() : TextureSamplerType());
            if (sampler == nullptr)
            {
                continue;
            }

            ID3D11ShaderResourceView* textureView = nullptr;
            const Texture_DirectX11* valueTexture = dynamic_cast<Texture_DirectX11*>(value);
            if (valueTexture != nullptr)
            {
                textureView = valueTexture->getTextureView();
            }
            else
            {
                const RenderTarget_DirectX11* valueRenderTarget = dynamic_cast<RenderTarget_DirectX11*>(value);
                if (valueRenderTarget != nullptr)
                {
                    textureView = valueRenderTarget->getResultImageView();
                }
                else if (defaultTexture != nullptr)
                {
                    textureView = defaultTexture->getTextureView();
                }
                else
                {
                    throw std::exception("Invalid default texture");
                }
            }
            if (textureView == nullptr)
            {
                JUTILS_LOG(error, JSTR("Failed to get DX11 texture view"));
                continue;
            }

            if (uniform.shaderStages & SHADER_STAGE_VERTEX)
            {
                deviceContext->VSSetSamplers(uniform.shaderLocation, 1, &sampler);
                deviceContext->VSSetShaderResources(uniform.shaderLocation, 1, &textureView);
            }
            if (uniform.shaderStages & SHADER_STAGE_FRAGMENT)
            {
                deviceContext->PSSetSamplers(uniform.shaderLocation, 1, &sampler);
                deviceContext->PSSetShaderResources(uniform.shaderLocation, 1, &textureView);
            }
        }
    }
    void Material_DirectX11::updateUniformBuffersData(ID3D11DeviceContext* deviceContext)
    {
        const jset<jstringID>& notUpdatedParams = getNotUpdatedParams();
        if (notUpdatedParams.isEmpty())
        {
            return;
        }

        jmap<uint32, D3D11_MAPPED_SUBRESOURCE> uniformBuffersData;
        const jmap<jstringID, ShaderUniform>& uniforms = getShader()->getUniforms();
        for (const auto& paramName : notUpdatedParams)
        {
            const ShaderUniform* uniformPtr = uniforms.find(paramName);
            const uint32 shaderLocation = uniformPtr != nullptr ? uniformPtr->shaderLocation : 0;
            if ((uniformPtr != nullptr) && !uniformBuffersData.contains(shaderLocation))
            {
                const UniformBufferDescription* bufferDescription = m_UniformBuffers.find(shaderLocation);
                if (bufferDescription != nullptr)
                {
                    D3D11_MAPPED_SUBRESOURCE& mappedData = uniformBuffersData.add(shaderLocation);
                    const HRESULT result = deviceContext->Map(bufferDescription->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
                    if (FAILED(result))
                    {
                        JUTILS_ERROR_LOG(result, JSTR("Failed to map DirectX11 uniform buffer data"));
                    }
                }
            }
        }

        const MaterialParamsStorage& materialParams = getMaterialParams();
        for (const auto& [uniformID, uniform] : uniforms)
        {
            const D3D11_MAPPED_SUBRESOURCE* mappedData = uniformBuffersData.find(uniform.shaderLocation);
            if (mappedData == nullptr)
            {
                continue;
            }

            switch (uniform.type)
            {
            case ShaderUniformType::Float:
                {
                    ShaderUniformInfo<ShaderUniformType::Float>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Float>(uniformID, value))
                    {
                        std::memcpy(static_cast<uint8*>(mappedData->pData) + uniform.shaderBlockOffset, &value, sizeof(value));
                    }
                }
                break;
            case ShaderUniformType::Vec2:
                {
                    ShaderUniformInfo<ShaderUniformType::Vec2>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Vec2>(uniformID, value))
                    {
                        std::memcpy(static_cast<uint8*>(mappedData->pData) + uniform.shaderBlockOffset, &value[0], sizeof(value));
                    }
                }
                break;
            case ShaderUniformType::Vec4:
                {
                    ShaderUniformInfo<ShaderUniformType::Vec4>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Vec4>(uniformID, value))
                    {
                        std::memcpy(static_cast<uint8*>(mappedData->pData) + uniform.shaderBlockOffset, &value[0], sizeof(value));
                    }
                }
                break;
            case ShaderUniformType::Mat4:
                {
                    ShaderUniformInfo<ShaderUniformType::Mat4>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Mat4>(uniformID, value))
                    {
                        std::memcpy(static_cast<uint8*>(mappedData->pData) + uniform.shaderBlockOffset, &value[0][0], sizeof(value));
                    }
                }
                break;
            default: ;
            }
        }
        for (const auto& bufferLocation : uniformBuffersData.keys())
        {
            deviceContext->Unmap(m_UniformBuffers[bufferLocation].buffer, 0);
        }

        clearParamsForUpdate();
    }
}

#endif
