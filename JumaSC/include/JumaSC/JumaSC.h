// Copyright © 2023 Leonov Maksim. All rights reserved.

#pragma once

#include <jutils/jstring.h>

namespace JumaSC
{
    namespace GLSL
    {
        enum class type : jutils::uint8 { vertex, fragment, geometry, tess_control, tess_evaluation, compute };
    }
    namespace Vulkan
    {
        enum class version : jutils::uint8 { _1_0, _1_1, _1_2, _1_3 };
    }
    namespace HLSL
    {
        enum class type : jutils::uint8 { vertex, fragment, geometry, hull, domain, compute };
        enum class model : jutils::uint8 { _6_0, _6_1, _6_2, _6_3, _6_4, _6_5, _6_6 };
    }

    class Compiler
    {
    protected:
        Compiler() = default;
    public:
        virtual ~Compiler() = default;

        static Compiler* Create();

        jutils::jarray<jutils::uint32> glslToSPV(const jutils::jarray<jutils::jstring>& shaderText, GLSL::type shaderType)
        {
            return glslToSPV(shaderText, shaderType, Vulkan::version::_1_3);
        }
        virtual jutils::jarray<jutils::uint32> glslToSPV(const jutils::jarray<jutils::jstring>& shaderText,
            GLSL::type shaderType, Vulkan::version vulkanVersion) = 0;
        virtual jutils::jstring hlslFromSPV(const jutils::jarray<jutils::uint32>& shaderData, HLSL::model shaderModel) = 0;
        virtual jutils::jarray<jutils::uint8> hlslCompile(const jutils::jarray<jutils::jstring>& shaderText,
            HLSL::type shaderType, HLSL::model shaderModel) = 0;
    };
}
