// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/vertex/VertexBuffer.h"

#include "../../include/JumaRE/vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    VertexBuffer::~VertexBuffer()
    {
        clearData();
    }

    bool VertexBuffer::init(VertexBufferData* verticesData)
    {
        m_VertexTypeName = verticesData->getVertexTypeName();
        if (!initInternal(verticesData))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize vertex buffer"));
            clearData();
            return false;
        }
        return true;
    }

    void VertexBuffer::clearData()
    {
        m_VertexTypeName = jstringID_NONE;
    }
}
