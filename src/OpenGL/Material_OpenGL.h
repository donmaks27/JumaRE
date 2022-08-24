// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "../../include/JumaRE/material/Material.h"

namespace JumaRenderEngine
{
    class Material_OpenGL final : public Material
    {
        using Super = Material;

    public:
        Material_OpenGL() = default;
        virtual ~Material_OpenGL() override;

        bool bindMaterial();
        void unbindMaterial();

    protected:

        virtual bool initInternal() override;

    private:

        jmap<uint32, uint32> m_UniformBufferIndices;


        void clearOpenGL();

        void updateUniformData();
    };
}

#endif
