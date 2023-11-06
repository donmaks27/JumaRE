// Copyright © 2023 Leonov Maksim. All rights reserved.

#include "CompilerInternal.h"

namespace JumaShaderCompiler
{
    Compiler* Compiler::Create()
    {
        return new CompilerInternal();
    }
}