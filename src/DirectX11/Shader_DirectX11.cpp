// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX11)

#include "Shader_DirectX11.h"

#include <d3d11.h>

#include "RenderEngine_DirectX11.h"
#include "VertexBuffer_DirectX11.h"

namespace JumaRenderEngine
{
    ID3DBlob* LoadDirectX11ShaderFile(const jstring& fileName, const bool optional)
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
        if (FAILED(result))
        {
            if (!optional)
            {
                JUTILS_ERROR_LOG(result, JSTR("Failed read shader file {} into DirectX11 blob"), fileName);
            }
            return nullptr;
        }

        return shaderBlob;
    }
    ID3DBlob* LoadDirectX11ShaderFile(const jmap<ShaderStageFlags, jstring>& fileNames, const ShaderStageFlags shaderStage, const bool optional)
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
        return LoadDirectX11ShaderFile(*fileName, optional);
    }

    Shader_DirectX11::~Shader_DirectX11()
    {
        clearDirectX();
    }

    bool Shader_DirectX11::initInternal(const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        ID3DBlob* vertexShaderBlob = LoadDirectX11ShaderFile(fileNames, SHADER_STAGE_VERTEX, false);
        if (vertexShaderBlob == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to load DirectX11 vertex shader"));
            return false;
        }
        ID3DBlob* fragmentShaderBlob = LoadDirectX11ShaderFile(fileNames, SHADER_STAGE_FRAGMENT, false);
        if (fragmentShaderBlob == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to load DirectX11 fragment shader"));
            vertexShaderBlob->Release();
            return false;
        }

        const RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        ID3D11Device* device = renderEngine->getDevice();

        ID3D11VertexShader* vertexShader = nullptr;
        HRESULT result = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX11 vertex shader"));
            fragmentShaderBlob->Release();
            vertexShaderBlob->Release();
            return false;
        }

        ID3D11PixelShader* fragmentShader = nullptr;
        result = device->CreatePixelShader(fragmentShaderBlob->GetBufferPointer(), fragmentShaderBlob->GetBufferSize(), nullptr, &fragmentShader);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX11 vertex shader"));
            vertexShader->Release();
            fragmentShaderBlob->Release();
            vertexShaderBlob->Release();
            return false;
        }

        fragmentShaderBlob->Release();

        m_VertexShaderBlob = vertexShaderBlob;
        m_VertexShader = vertexShader;
        m_FragmentShader = fragmentShader;
        return true;
    }

    void Shader_DirectX11::onClearAsset()
    {
        clearDirectX();
        Super::onClearAsset();
    }
    void Shader_DirectX11::clearDirectX()
    {
        for (const auto& inputLayout : m_VertexInputLayouts)
        {
            inputLayout.value->Release();
        }
        m_VertexInputLayouts.clear();

        if (m_FragmentShader != nullptr)
        {
            m_FragmentShader->Release();
            m_FragmentShader = nullptr;
        }
        if (m_VertexShader != nullptr)
        {
            m_VertexShader->Release();
            m_VertexShader = nullptr;
        }
        if (m_VertexShaderBlob != nullptr)
        {
            m_VertexShaderBlob->Release();
            m_VertexShaderBlob = nullptr;
        }
    }

    ID3D11InputLayout* Shader_DirectX11::getVertexInputLayout(const vertex_id vertexID)
    {
        ID3D11InputLayout** existingInputLayoutPtr = m_VertexInputLayouts.find(vertexID);
        if (existingInputLayoutPtr != nullptr)
        {
            return *existingInputLayoutPtr;
        }

        if (m_VertexShaderBlob == nullptr)
        {
            return nullptr;
        }
        const RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        const RegisteredVertexDescription* vertexDescription = renderEngine->findVertex(vertexID);
        if (vertexDescription == nullptr)
        {
            return nullptr;
        }

        uint32 componentOffset = 0;
        jarray<jstring> semanticNames;
        jarray<D3D11_INPUT_ELEMENT_DESC> vertexLayoutDescriptions;
        vertexLayoutDescriptions.reserve(vertexDescription->description.components.getSize());
        for (const auto& componentID : vertexDescription->description.components)
        {
            const VertexComponentDescription* componentDescription = renderEngine->findVertexComponent(componentID);
            DXGI_FORMAT componentFormat;
            switch (componentDescription->type)
            {
            case VertexComponentType::Float: componentFormat = DXGI_FORMAT_R32_FLOAT; break;
            case VertexComponentType::Vec2: componentFormat = DXGI_FORMAT_R32G32_FLOAT; break;
            case VertexComponentType::Vec3: componentFormat = DXGI_FORMAT_R32G32B32_FLOAT; break;
            case VertexComponentType::Vec4: componentFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
            default: 
                JUTILS_LOG(error, JSTR("Unsupported vertex component type!"));
                continue;
            }

            vertexLayoutDescriptions.add({
                JSTR("TEXCOORD"), componentDescription->shaderLocation, componentFormat, 0, componentOffset, D3D11_INPUT_PER_VERTEX_DATA, 0
            });
            componentOffset += GetVertexComponentSize(componentDescription->type);
        }

        ID3D11InputLayout* inputLayout = nullptr;
        const HRESULT result = renderEngine->getDevice()->CreateInputLayout(
            vertexLayoutDescriptions.getData(), vertexLayoutDescriptions.getSize(), 
            m_VertexShaderBlob->GetBufferPointer(), m_VertexShaderBlob->GetBufferSize(), 
            &inputLayout
        );
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX11 input layout for vertex {}"), vertexID);
            return nullptr;
        }
        return m_VertexInputLayouts.add(vertexID, inputLayout);
    }

    bool Shader_DirectX11::bindShader(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer)
    {
        ID3D11InputLayout* inputLayout = getVertexInputLayout(vertexBuffer->getVertexID());
        if (inputLayout == nullptr)
        {
            return false;
        }

        ID3D11DeviceContext* deviceContext = getRenderEngine<RenderEngine_DirectX11>()->getDeviceContext();
        deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
        deviceContext->PSSetShader(m_FragmentShader, nullptr, 0);
        deviceContext->IASetInputLayout(inputLayout);
        return true;
    }
    void Shader_DirectX11::unbindShader(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer)
    {
        ID3D11DeviceContext* deviceContext = getRenderEngine<RenderEngine_DirectX11>()->getDeviceContext();
        deviceContext->IASetInputLayout(nullptr);
        deviceContext->PSSetShader(nullptr, nullptr, 0);
        deviceContext->VSSetShader(nullptr, nullptr, 0);
    }
}

#endif
