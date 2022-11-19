// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

namespace JumaRenderEngine
{
    struct MaterialProperties
    {
        bool depthEnabled = true;
        bool stencilEnabled = false;
        bool wireframe = false;
        bool cullBackFaces = true;
        bool blendEnabled = true;
        // blend type

        constexpr bool operator==(const MaterialProperties& properties) const
        {
            return (depthEnabled == properties.depthEnabled)
                && (stencilEnabled == properties.stencilEnabled)
                && (wireframe == properties.wireframe)
                && (cullBackFaces == properties.cullBackFaces)
                && (blendEnabled == properties.blendEnabled);
        }
        constexpr bool operator<(const MaterialProperties& properties) const
        {
            if (depthEnabled != properties.depthEnabled)
            {
                return !depthEnabled;
            }
            if (stencilEnabled != properties.stencilEnabled)
            {
                return !stencilEnabled;
            }
            if (wireframe != properties.wireframe)
            {
                return !wireframe;
            }
            if (cullBackFaces != properties.cullBackFaces)
            {
                return !cullBackFaces;
            }
            return !cullBackFaces && properties.cullBackFaces;
        }
    };
}
