// Copyright © 2023 Leonov Maksim. All rights reserved.

#pragma once

#include "JumaShaderCompiler/Compiler.h"

#ifdef JUMASC_ENABLE_DXC

#include <dxc/dxcapi.h>

namespace JumaShaderCompiler
{
    struct CompilerPart_dxc
    {
        CComPtr<IDxcCompiler3> compiler = nullptr;
        CComPtr<IDxcUtils> utils = nullptr;
    };

    namespace HLSL
    {
        constexpr std::wstring GetHLSLTargetProfile(const HLSL::type type, const HLSL::model model)
        {
            std::wstring result;
            switch (type)
            {
            case type::vertex:   result += L"vs"; break;
            case type::fragment: result += L"ps"; break;
            case type::geometry: result += L"gs"; break;
            case type::hull:     result += L"hs"; break;
            case type::domain:   result += L"ds"; break;
            case type::compute:  result += L"cs"; break;
            default: return {};
            }
            switch (model)
            {
            case model::_6_0: result += L"_6_0"; break;
            case model::_6_1: result += L"_6_1"; break;
            case model::_6_2: result += L"_6_2"; break;
            case model::_6_3: result += L"_6_3"; break;
            case model::_6_4: result += L"_6_4"; break;
            case model::_6_5: result += L"_6_5"; break;
            case model::_6_6: result += L"_6_6"; break;
            default: return {};
            }
            return result;
        }
    }
}

#else

namespace JumaShaderCompiler
{
    struct CompilerPart_dxc {};
}

#endif
