// Copyright © 2023 Leonov Maksim. All rights reserved.

#pragma once

#include "JumaShaderCompiler/Compiler.h"

#ifdef JUMASC_ENABLE_GLSLANG

namespace JumaShaderCompiler
{
    struct CompilerPart_glslang
    {
        ~CompilerPart_glslang();

        bool initialized = false;
    };
}

#else

namespace JumaShaderCompiler
{
    struct CompilerPart_glslang {};
}

#endif