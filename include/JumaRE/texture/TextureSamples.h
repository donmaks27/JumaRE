// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

namespace JumaRenderEngine
{
    enum class TextureSamples : uint8
    {
        X1, X2, X4, X8, X16
    };

    constexpr uint8 GetTextureSamplesNumber(const TextureSamples samples)
    {
        switch (samples)
        {
        case TextureSamples::X1:  return 1;
        case TextureSamples::X2:  return 2;
        case TextureSamples::X4:  return 4;
        case TextureSamples::X8:  return 8;
        case TextureSamples::X16: return 16;
        default: ;
        }
        return 0;
    }
}
