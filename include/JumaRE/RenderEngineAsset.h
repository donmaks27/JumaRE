// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"
#include "RenderEngineContextObject.h"

#include <jutils/jdelegate_multicast.h>

namespace JumaRenderEngine
{
    class RenderEngineAsset;

    JUTILS_CREATE_MULTICAST_DELEGATE1(OnRenderEngineAssetEvent, RenderEngineAsset*, asset);

    class RenderEngineAsset : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        RenderEngineAsset() = default;
        virtual ~RenderEngineAsset() override = default;

        OnRenderEngineAssetEvent onDestroying;

    protected:

        virtual void onClearAsset() {}

    private:

        void clearAsset();
    };
}
