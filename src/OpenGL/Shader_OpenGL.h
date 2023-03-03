// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "JumaRE/material/Shader.h"

namespace JumaRenderEngine
{
    class Shader_OpenGL final : public Shader
    {
        using Super = Shader;

    public:
        Shader_OpenGL() = default;
        virtual ~Shader_OpenGL() override;

        bool activateShader() const;
        static void deactivateAnyShader();

    protected:

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) override;
        virtual void onClearAsset() override;

    private:

        uint32 m_ShaderProgramIndex = 0;


        void clearOpenGL();
    };
}

#endif
