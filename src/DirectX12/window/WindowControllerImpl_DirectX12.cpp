// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12)

#include "WindowControllerImpl_DirectX12.h"

#include "WindowController_DirectX12_GLFW.h"

namespace JumaRenderEngine
{
    WindowController_DirectX12* CreateWindowController_DirectX12()
    {
#if defined(JUMARE_ENABLE_LIB_GLFW)
        return new WindowController_DirectX12_GLFW();
#else
        static_assert(false, "There is no implementation of WindowController_DirectX12 class");
#endif
    }
}

#endif
