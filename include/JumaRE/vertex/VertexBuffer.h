// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"
#include "../RenderEngineAsset.h"

#include <jutils/jstringID.h>

namespace JumaRenderEngine
{
    struct RenderOptions;
    class Material;
    class VertexBufferData;

    class VertexBuffer : public RenderEngineAsset
    {
        friend RenderEngine;

    public:
        VertexBuffer() = default;
        virtual ~VertexBuffer() override;

        const jstringID& getVertexTypeName() const { return m_VertexTypeName; }

        virtual void render(const RenderOptions* renderOptions, Material* material) = 0;

    protected:

        bool init(VertexBufferData* verticesData);

        virtual bool initInternal(VertexBufferData* verticesData) = 0;
        virtual void clearAsset() override { clearData(); }

    private:

        jstringID m_VertexTypeName = jstringID_NONE;


        void clearData();
    };
}
