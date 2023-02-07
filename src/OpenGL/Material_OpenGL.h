// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "../../include/JumaRE/material/Material.h"

namespace JumaRenderEngine
{
	struct RenderOptions;

	class Material_OpenGL final : public Material
    {
        using Super = Material;

    public:
        Material_OpenGL() = default;
        virtual ~Material_OpenGL() override;

        bool bindMaterial(const RenderOptions* renderOptions);
        void unbindMaterial();

    protected:

        virtual bool initInternal() override;
        virtual void clearAsset() override;

    private:

        jmap<uint32, uint32> m_UniformBufferIndices;


        void clearOpenGL();

        void updateUniformData();
    };
}

#endif
