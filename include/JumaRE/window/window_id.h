﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

#include <jutils/juid.h>

namespace JumaRenderEngine
{
    using window_id = uint32;
    constexpr window_id window_id_INVALID = juid<window_id>::invalidUID;
}
