// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngineContextObject.h"

namespace JumaRenderEngine
{
    class RenderEngineAsset : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        RenderEngineAsset() = default;
        virtual ~RenderEngineAsset() override = default;

    protected:

        virtual void clearAsset() {}
    };
}
