// Copyright © 2023 Leonov Maksim. All rights reserved.

#include "CompilerInternal.h"

#ifdef JUMASC_ENABLE_SPIRV_CROSS

#include <spirv_cross/spirv_hlsl.hpp>

namespace JumaSC
{
    jstring CompilerInternal::hlslFromSPV(const jarray<uint32>& shaderData, const HLSL::model shaderModel)
    {
        spirv_cross::CompilerHLSL hlsl(shaderData.toBase());
        spirv_cross::CompilerHLSL::Options options{};
        switch (shaderModel)
        {
        case HLSL::model::_6_0: options.shader_model = 60; break;
        case HLSL::model::_6_1: options.shader_model = 61; break;
        case HLSL::model::_6_2: options.shader_model = 62; break;
        case HLSL::model::_6_3: options.shader_model = 63; break;
        case HLSL::model::_6_4: options.shader_model = 64; break;
        case HLSL::model::_6_5: options.shader_model = 65; break;
        case HLSL::model::_6_6: options.shader_model = 66; break;
        default:
            JUTILS_LOG(error, "invalid HLSL model");
            return {};
        }
        hlsl.set_hlsl_options(options);
        return hlsl.compile();
    }
}

#else

namespace JumaSC
{
    jstring CompilerInternal::hlslFromSPV(const jarray<uint32>&, HLSL::model)
    {
        JUTILS_LOG(error, "converting SPIR-V to HLSL disabled");
        return {};
    }
}

#endif