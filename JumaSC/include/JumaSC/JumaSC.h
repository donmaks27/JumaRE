// Copyright © 2023 Leonov Maksim. All rights reserved.

#pragma once

#include <jutils/jstring.h>

namespace JumaSC
{
    namespace HLSL
    {
        enum class type : jutils::uint8 { vertex, fragment, geometry, hull, domain, compute };
        enum class model : jutils::uint8 { _5_0, _5_1, _6_0, _6_1, _6_2, _6_3, _6_4, _6_5, _6_6 };

        extern jutils::jarray<jutils::uint8> compile(const jutils::jarray<jutils::jstring>& shaderText, type shaderType, model shaderModel);
    }
}
