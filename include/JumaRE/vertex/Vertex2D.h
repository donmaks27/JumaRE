// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

#include <jutils/math/vector2.h>

#include "VertexInfo.h"

namespace JumaRenderEngine
{
    struct Vertex2D
    {
        math::vector2 position;
    };

    template<>
    struct VertexInfo<Vertex2D> : std::true_type
    {
        JUMA_RENDER_ENGINE_VERTEX_TYPE(Vertex2D)

        static jarray<VertexComponentDescription> getVertexComponents()
        {
            return {{ JSTR("position"), VertexComponentType::Vec2, 0, offsetof(VertexType, position) }};
        }
    };
}
