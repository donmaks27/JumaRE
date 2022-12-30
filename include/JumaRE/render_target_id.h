// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"

#include <jutils/juid.h>

namespace JumaRenderEngine
{
    using render_target_id = uint32;
    constexpr render_target_id render_target_id_INVALID = juid<render_target_id>::invalidUID;
}
