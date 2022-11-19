// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "../../include/JumaRE/vertex/VertexBuffer.h"

#include <jutils/jmap.h>

#include "../../include/JumaRE/window/window_id.h"

namespace JumaRenderEngine
{
    class VertexBuffer_OpenGL final : public VertexBuffer
    {
        using Super = VertexBuffer;

    public:
        VertexBuffer_OpenGL() = default;
        virtual ~VertexBuffer_OpenGL() override;

        virtual void render(const RenderOptions* renderOptions, Material* material) override;

    protected:

        virtual bool initInternal(const VertexBufferData& data) override;
        virtual void clearAsset() override;

    private:

        uint32 m_VerticesBufferIndex = 0;
        uint32 m_IndicesBufferIndex = 0;
        jmap<window_id, uint32> m_VertexArrayIndices;

        int32 m_RenderElementsCount = 0;


        void clearOpenGL();

        uint32 getVerticesVAO(window_id windowID);
        uint32 createVerticesVAO() const;
    };
}

#endif
