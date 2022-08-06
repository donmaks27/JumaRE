// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL)

#include "WindowControllerImpl_Vulkan.h"

#include "WindowController_Vulkan_GLFW.h"

namespace JumaRenderEngine
{
    WindowController_Vulkan* CreateWindowController_Vulkan()
    {
#if defined(JUMARE_ENABLE_VULKAN_GLFW)
        return new WindowController_Vulkan_GLFW();
#else
        return nullptr;
#endif
    }
}

#endif
