// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/RenderEngineImpl_DirectX12.h"

#include "RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
	bool IsSupportRenderAPI_DirectX12()
	{
#if defined(JUMARE_ENABLE_DX12)
        return true;
#else
        return false;
#endif
	}

    RenderEngine* CreateRenderEngine_DirectX12()
    {
#if defined(JUMARE_ENABLE_DX12)
        return new RenderEngine_DirectX12();
#else
        return nullptr;
#endif
    }
}
