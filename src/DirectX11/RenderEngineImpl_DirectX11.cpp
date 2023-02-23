// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/RenderEngineImpl_DirectX11.h"

#include "RenderEngine_DirectX11.h"

namespace JumaRenderEngine
{
	bool IsSupportRenderAPI_DirectX11()
	{
#if defined(JUMARE_ENABLE_DX11)
        return true;
#else
        return false;
#endif
	}

    RenderEngine* CreateRenderEngine_DirectX11()
    {
#if defined(JUMARE_ENABLE_DX11)
        return new RenderEngine_DirectX11();
#else
        return nullptr;
#endif
    }
}
