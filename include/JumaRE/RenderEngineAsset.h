// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"
#include "RenderEngineContextObject.h"

#include <jutils/jdelegate_multicast.h>

namespace JumaRenderEngine
{
    class RenderEngineAsset;

    enum class RenderEngineAssetType : uint8 { None, Shader, Material, Texture, RenderTarget, VertexBuffer };

    JUTILS_CREATE_MULTICAST_DELEGATE1(OnRenderEngineAssetEvent, RenderEngineAsset*, asset);

    class RenderEngineAsset : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    protected:
        RenderEngineAsset() = default;
        RenderEngineAsset(const RenderEngineAssetType type) : m_AssetType(type) {}
    public:
        virtual ~RenderEngineAsset() override = default;

        OnRenderEngineAssetEvent onDestroying;


        RenderEngineAssetType getType() const { return m_AssetType; }

    protected:

        virtual void onClearAsset() {}

    private:

        RenderEngineAssetType m_AssetType = RenderEngineAssetType::None;


        void clearAsset();
    };
}
