// Copyright © 2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

#include <jutils/jmap.h>
#include <jutils/jset.h>
#include <jutils/jstringID.h>

#include "ShaderUniform.h"

namespace JumaRenderEngine
{
	struct ShaderCreateInfo
    {
        jmap<ShaderStageFlags, jstring> fileNames;
        jset<jstringID> vertexComponents;
        jmap<jstringID, ShaderUniform> uniforms;
    };
}
