// Copyright © 2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"

#include <jutils/jarray.h>

namespace JumaRenderEngine
{
	class Material;
	class VertexBuffer;

	struct RenderPrimitive
    {
        VertexBuffer* vertexBuffer = nullptr;
        Material* material = nullptr;
    };
    struct RenderStageProperties
    {
        bool depthEnabled = true;
    };
    struct RenderStage
    {
	    jarray<RenderPrimitive> primitivesList;
        RenderStageProperties properties;
    };
}
