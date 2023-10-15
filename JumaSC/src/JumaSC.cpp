// Copyright © 2023 Leonov Maksim. All rights reserved.

#include "JumaSC/JumaSC.h"

#include <dxc/dxcapi.h>

#include <jutils/log.h>
#include <jutils/jdefer.h>
using namespace jutils;

namespace JumaSC
{
    constexpr std::wstring GetHLSLTargetProfile(const HLSL::type type, const HLSL::model model)
    {
        std::wstring result;
        switch (type)
        {
        case HLSL::type::vertex:   result += L"vs"; break;
        case HLSL::type::fragment: result += L"ps"; break;
        case HLSL::type::geometry: result += L"gs"; break;
        case HLSL::type::hull:     result += L"hs"; break;
        case HLSL::type::domain:   result += L"ds"; break;
        case HLSL::type::compute:  result += L"cs"; break;
        default: return {};
        }
        switch (model)
        {
        case HLSL::model::_5_0: result += L"_5_0"; break;
        case HLSL::model::_5_1: result += L"_5_1"; break;
        case HLSL::model::_6_0: result += L"_6_0"; break;
        case HLSL::model::_6_1: result += L"_6_1"; break;
        case HLSL::model::_6_2: result += L"_6_2"; break;
        case HLSL::model::_6_3: result += L"_6_3"; break;
        case HLSL::model::_6_4: result += L"_6_4"; break;
        case HLSL::model::_6_5: result += L"_6_5"; break;
        case HLSL::model::_6_6: result += L"_6_6"; break;
        default: return {};
        }
        return result;
    }

    void printShaderText(const jarray<jstring>& shaderText)
    {
        for (int32 index = 0; index < shaderText.getSize(); index++)
        {
            log::print(JSTR_FORMAT("{} {:>3}: {}\n", log::verbosityLevel::warning, index + 1, shaderText[index]));
        }
    }

    jarray<uint8> HLSL::compile(const jutils::jarray<jutils::jstring>& shaderText, type shaderType, model shaderModel)
    {
        CComPtr<IDxcCompiler3> dxcCompiler = nullptr;
        CComPtr<IDxcUtils> dxcUtils = nullptr;
        HRESULT result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, "failed to create DXC compiler");
            return {};
        }
        result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, "failed to create DXC instance");
            return {};
        }

        const jstring fullShaderText = string::join(shaderText, '\n');
        CComPtr<IDxcBlobEncoding> dxcSourceBlob = nullptr;
        result = dxcUtils->CreateBlob(fullShaderText.getData(), fullShaderText.getSize(), CP_UTF8, &dxcSourceBlob);
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
        result = dxcCompiler->Compile(
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