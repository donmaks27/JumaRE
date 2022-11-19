// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

#include <jutils/jstringID.h>
#include <jutils/juid.h>

namespace JumaRenderEngine
{
    enum class VertexComponentType : uint8
    {
        Float,
        Vec2,
        Vec3,
        Vec4
    };
    constexpr uint8 GetVertexComponentSize(const VertexComponentType type)
    {
        switch (type)
        {
        case VertexComponentType::Float: return 4;
        case VertexComponentType::Vec2: return 8;
        case VertexComponentType::Vec3: return 12;
        case VertexComponentType::Vec4: return 16;
        default: ;
        }
        return 0;
    }

    struct VertexComponentDescription
    {
        VertexComponentType type = VertexComponentType::Float;
        uint32 shaderLocation = 0;
    };
    struct VertexDescription
    {
        jarray<jstringID> components;

        bool operator<(const VertexDescription& description) const
        {
            if (components.getSize() < description.components.getSize())
            {
                return true;
            }
            if (components.getSize() > description.components.getSize())
            {
                return false;
            }
            for (int32 index = 0; index < components.getSize(); index++)
            {
                const jstringID& componentID = components[index];
                const jstringID& otherComponentID = description.components[index];
                if (componentID < otherComponentID)
                {
                    return true;
                }
                if (componentID > otherComponentID)
                {
                    return false;
                }
            }
            return false;
        }
    };

    using vertex_id = uint16;
    constexpr vertex_id vertex_id_NONE = juid<vertex_id>::invalidUID;
}
