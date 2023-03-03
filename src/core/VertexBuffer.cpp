// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/vertex/VertexBuffer.h"

#include "../../include/JumaRE/vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    VertexBuffer::~VertexBuffer()
    {
        clearData();
    }

    bool VertexBuffer::init(const vertex_id vertexID, const VertexBufferData& data)
    {
        m_VertexID = vertexID;
        if (!initInternal(data))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize vertex buffer"));
            clearData();
            return false;
        }
        return true;
    }
    
    void VertexBuffer::onClearAsset()
    {
	    clearData();
        Super::onClearAsset();
    }
    void VertexBuffer::clearData()
    {
        m_VertexID = vertex_id_NONE;
    }
}
