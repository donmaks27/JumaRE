// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

namespace JumaRenderEngine
{
    enum class WindowMode : uint8
    {
        Normal,
        Fullscreen,
        WindowedFullscreen
    };

    enum class WindowCursorMode : uint8
    {
        None,
        Normal,  // Normal cursor
        Hidden,  // Like normal, but visually hidden
        Locked,  // Cursor hidden, disabled direct position update, handle cursor offset
        Disabled // Cursor hidden, input ignored
    };
}
