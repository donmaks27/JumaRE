// Copyright © 2023 Leonov Maksim. All rights reserved.

#include "CompilerInternal.h"

#ifdef JUMASC_ENABLE_DXC

namespace JumaShaderCompiler
{
    bool CreateDXCInstances(CompilerPart_dxc& outData)
    {
        if (outData.compiler == nullptr)
        {
            const HRESULT result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&outData.compiler));
            if (FAILED(result) || (outData.compiler == nullptr))
            {
                JUTILS_ERROR_LOG(result, "failed to create DXC compiler");
                return false;
            }
        }
        if (outData.utils == nullptr)
        {
            const HRESULT result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&outData.utils));
            if (FAILED(result) || (outData.utils == nullptr))
            {
                outData.compiler.Release();
                JUTILS_ERROR_LOG(result, "failed to create DXC instance");
                return false;
            }
        }
        return true;
    }

    jarray<uint8> CompilerInternal::hlslCompile(const jarray<jstring> &shaderText, HLSL::type shaderType, HLSL::model shaderModel)
    {
        if (!CreateDXCInstances(dxcPart))
        {
            JUTILS_LOG(error, "failed to create DXC instances");
            return {};
        }

        const jstring fullShaderText = string::join(shaderText, '\n');
        CComPtr<IDxcBlobEncoding> dxcSourceBlob = nullptr;
        HRESULT result = dxcPart.utils->CreateBlob(fullShaderText.getData(), fullShaderText.getSize(), CP_UTF8, &dxcSourceBlob);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, "failed to create DXC shader source blob");
            return {};
        }
        DxcBuffer dxcSourceBuffer{};
        dxcSourceBuffer.Ptr = dxcSourceBlob->GetBufferPointer();
        dxcSourceBuffer.Size = dxcSourceBlob->GetBufferSize();
        dxcSourceBuffer.Encoding = 0;

        const std::wstring shaderProfile = GetHLSLTargetProfile(shaderType, shaderModel);
        jarray<LPCWSTR> dxcCompilerArgs = {
            L"-E", L"main",
            L"-T", shaderProfile.c_str(),
            L"-Qstrip_debug", L"-Qstrip_reflect",
            DXC_ARG_WARNINGS_ARE_ERRORS
#ifdef JUMASC_DEBUG
            , DXC_ARG_DEBUG
#endif
        };
        CComPtr<IDxcResult> dxcCompileResult;
        result = dxcPart.compiler->Compile(
            &dxcSourceBuffer,
            dxcCompilerArgs.getData(), dxcCompilerArgs.getSize(), nullptr,
            IID_PPV_ARGS(&dxcCompileResult)
        );
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, "failed to compile HLSL shader with DXC");
            return {};
        }
        CComPtr<IDxcBlobUtf8> dxcErrorsBlob = nullptr;
        result = dxcCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&dxcErrorsBlob), nullptr);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, "failed to get DXC compile errors");
            return {};
        }
        if ((dxcErrorsBlob != nullptr) && (dxcErrorsBlob->GetStringLength() > 0))
        {
            JUTILS_LOG(error, "DXC compile error: {}", reinterpret_cast<const char*>(dxcErrorsBlob->GetBufferPointer()));
            printShaderText(shaderText);
            return {};
        }

        CComPtr<IDxcBlob> dxcObjectBlob = nullptr;
        result = dxcCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxcObjectBlob), nullptr);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, "failed to get compiled shader object");
            return {};
        }
        jarray<uint8> compiledData(static_cast<int32>(dxcObjectBlob->GetBufferSize()));
        std::memcpy(compiledData.getData(), dxcObjectBlob->GetBufferPointer(), compiledData.getSize());
        return compiledData;
    }
}

#else

namespace JumaShaderCompiler
{
    jarray<uint8> CompilerInternal::hlslCompile(const jarray<jstring> &shaderText, HLSL::type shaderType, HLSL::model shaderModel)
    {
        JUTILS_LOG(warning, "compiling HLSL shaders is disabled");
        return {};
    }
}

#endif