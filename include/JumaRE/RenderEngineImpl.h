// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngineImpl_OpenGL.h"
#include "RenderEngineImpl_Vulkan.h"
#include "RenderEngineImpl_DirectX11.h"
#include "RenderEngineImpl_DirectX12.h"

namespace JumaRenderEngine
{
    inline bool IsSupportRenderAPI(const RenderAPI api)
    {
	    switch (api)
	    {
	    case RenderAPI::Vulkan: return IsSupportRenderAPI<RenderAPI::Vulkan>();
	    case RenderAPI::OpenGL: return IsSupportRenderAPI<RenderAPI::OpenGL>();
	    case RenderAPI::DirectX11: return IsSupportRenderAPI<RenderAPI::DirectX11>();
	    case RenderAPI::DirectX12: return IsSupportRenderAPI<RenderAPI::DirectX12>();
	    default: ;
	    }
        return false;
    }

    inline RenderEngine* CreateRenderEngine(const RenderAPI api)
    {
        switch (api)
        {
        case RenderAPI::Vulkan: return CreateRenderEngine<RenderAPI::Vulkan>();
        case RenderAPI::OpenGL: return CreateRenderEngine<RenderAPI::OpenGL>();
        case RenderAPI::DirectX11: return CreateRenderEngine<RenderAPI::DirectX11>();
        case RenderAPI::DirectX12: return CreateRenderEngine<RenderAPI::DirectX12>();
        default: ;
        }
        return nullptr;
    }
}
