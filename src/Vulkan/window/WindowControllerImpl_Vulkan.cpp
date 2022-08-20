// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "WindowControllerImpl_Vulkan.h"

#include "WindowController_Vulkan_GLFW.h"

namespace JumaRenderEngine
{
    WindowController_Vulkan* CreateWindowController_Vulkan()
    {
#if defined(JUMARE_ENABLE_LIB_GLFW)
        return new WindowController_Vulkan_GLFW();
#else
        static_assert(false, "There is no implementation of WindowController_Vulkan class");
#endif
    }
}

#endif
