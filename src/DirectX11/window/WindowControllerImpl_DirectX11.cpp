// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX11)

#include "WindowControllerImpl_DirectX11.h"

#include "WindowController_DirectX11_GLFW.h"

namespace JumaRenderEngine
{
    WindowController_DirectX11* CreateWindowController_DirectX11()
    {
#if defined(JUMARE_ENABLE_LIB_GLFW)
        return new WindowController_DirectX11_GLFW();
#else
        static_assert(false, "There is no implementation of WindowController_DirectX11 class");
#endif
    }
}

#endif
