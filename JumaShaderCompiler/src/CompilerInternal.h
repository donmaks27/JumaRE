// Copyright © 2023 Leonov Maksim. All rights reserved.

#pragma once

#include "JumaSC_dxc.h"
#include "JumaSC_glslang.h"

#include <jutils/log.h>
using namespace jutils;

namespace JumaShaderCompiler
{
    class CompilerInternal : public Compiler
    {
    public:
        CompilerInternal() = default;
        virtual ~CompilerInternal() override = default;

#ifdef JUMASC_ENABLE_GLSLANG
        virtual bool isGlslCompileEnabled() const override { return true; }
#endif
#ifdef JUMASC_ENABLE_SPIRV_CROSS
        virtual bool isSpvToHlslEnabled() const override { return true; }
#endif
#ifdef JUMASC_ENABLE_DXC
        virtual bool isHlslCompileEnabled() const override { return true; }
#endif

        virtual jarray<uint32> glslToSPV(const jarray<jstring>& shaderText, GLSL::type shaderType, Vulkan::version vulkanVersion) override;
        virtual jstring hlslFromSPV(const jarray<uint32>& shaderData, HLSL::model shaderModel) override;
        virtual jarray<uint8> hlslCompile(const jarray<jstring> &shaderText, HLSL::type shaderType, HLSL::model shaderModel) override;

    private:

        CompilerPart_dxc dxcPart;
        CompilerPart_glslang glslangPart;
    };

    inline void printShaderText(const jarray<jstring>& shaderText)
    {
        for (int32 index = 0; index < shaderText.getSize(); index++)
        {
            log::print(JSTR_FORMAT("{} {:>3}: {}\n", log::verbosityLevel::warning, index + 1, shaderText[index]));
        }
    }
}
