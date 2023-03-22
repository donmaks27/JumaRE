// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"
#include "../RenderEngineAsset.h"

#include "VertexBufferData.h"

namespace JumaRenderEngine
{
    struct RenderOptions;
    class Material;

    class VertexBuffer : public RenderEngineAsset
    {
        friend RenderEngine;

        using Super = RenderEngineAsset;

    public:
        VertexBuffer() : Super(RenderEngineAssetType::VertexBuffer) {}
        virtual ~VertexBuffer() override;

        vertex_id getVertexID() const { return m_VertexID; }

        virtual void render(const RenderOptions* renderOptions, Material* material) = 0;

    protected:

        virtual bool initInternal(const VertexBufferData& data) = 0;
        virtual void onClearAsset() override;

    private:

        vertex_id m_VertexID = vertex_id_NONE;


        bool init(vertex_id vertexID, const VertexBufferData& data);

        void clearData();
    };
}
