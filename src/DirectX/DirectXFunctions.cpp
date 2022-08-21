// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX)

#include "DirectXFunctions.h"

#include <dxgi1_5.h>

namespace JumaRenderEngine
{
    bool DirectX_IsTearingSupported()
    {
        static bool alreadyGetValue = false;
        static bool tearingSupported = false;

        if (!alreadyGetValue)
        {
            IDXGIFactory4* factory4 = nullptr;
#if defined(JDEBUG)
            constexpr UINT createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
            constexpr UINT createFactoryFlags = 0;
#endif
            HRESULT result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory4));
            if (FAILED(result))
            {
                return false;
            }

            IDXGIFactory5* factory5 = nullptr;
            result = factory4->QueryInterface(&factory5);
            factory4->Release();
            if (FAILED(result))
            {
                return false;
            }

            BOOL allowTearing = FALSE;
            factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            factory5->Release();

            tearingSupported = allowTearing == TRUE;
            alreadyGetValue = true;
        }
        return tearingSupported;
    }
}

#endif
