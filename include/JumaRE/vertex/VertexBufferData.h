// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

#include <jutils/jarray.h>

#include "VertexDescription.h"

namespace JumaRenderEngine
{
    struct VertexBufferData
    {
        VertexDescription vertexDescription;

        const void* verticesData = nullptr;
        const uint32* indicesData = nullptr;

        uint32 vertexCount = 0;
        uint32 indexCount = 0;
    };
    template<typename T>
    VertexBufferData MakeVertexBufferData(const VertexDescription& description, const jarray<T>& vertices, const jarray<uint32>& indices)
    {
        return {
            description, vertices.getData(), indices.getData(),
            static_cast<uint32>(vertices.getSize()), static_cast<uint32>(indices.getSize())
        };
    }
    template<typename T>
    VertexBufferData MakeVertexBufferData(const VertexDescription& description, const jarray<T>& vertices)
    {
        return {
            description, vertices.getData(), nullptr,
            static_cast<uint32>(vertices.getSize()), 0
        };
    }
}
