// Copyright © 2023 Leonov Maksim. All rights reserved.

#pragma once

#include "JumaSC/JumaSC.h"

#ifdef JUMASC_ENABLE_GLSLANG

namespace JumaSC
{
    struct CompilerPart_glslang
    {
        ~CompilerPart_glslang();

        bool initialized = false;
    };
}

#else

namespace JumaSC
{
    struct CompilerPart_glslang {};
}

#endif