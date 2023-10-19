// Copyright © 2023 Leonov Maksim. All rights reserved.

#pragma once

#include "JumaSC_dxc.h"
#include "JumaSC_glslang.h"

#include <jutils/log.h>
using namespace jutils;

namespace JumaSC
{
    class CompilerInternal : public Compiler
    {
    public:
        CompilerInternal() = default;
        virtual ~CompilerInternal() override = default;

        virtual jarray<uint32> glslToSPV(const jarray<jstring>& shaderText, GLSL::type shaderType, Vulkan::version vulkanVersion) override;
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
