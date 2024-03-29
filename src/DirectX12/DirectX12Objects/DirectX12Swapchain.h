﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "JumaRE/RenderEngineContextObject.h"

#include <jutils/jarray.h>

#include "DirectX12Texture.h"
#include "JumaRE/window/window_id.h"

struct IDXGISwapChain4;

namespace JumaRenderEngine
{
    class DirectX12Swapchain;
    class WindowController;
    class WindowController_DirectX12;
    struct WindowData;

    class DirectX12Swapchain : public RenderEngineContextObjectBase
    {
        friend WindowController_DirectX12;

    public:
        DirectX12Swapchain() = default;
        virtual ~DirectX12Swapchain() override;

        window_id getWindowID() const { return m_WindowID; }

        IDXGISwapChain4* get() const { return m_Swapchain; }

        DirectX12Texture* getBuffer(const uint8 bufferIndex) const { return m_SwapchainBuffers.isValidIndex(bufferIndex) ? &m_SwapchainBuffers[bufferIndex] : nullptr; }
        uint8 getBuffersCount() const { return static_cast<uint8>(m_SwapchainBuffers.getSize()); }
        uint8 getCurrentBufferIndex() const { return m_CurrentBufferIndex; }

        math::uvector2 getBuffersSize() const { return m_SwapchainBuffersSize; }

        bool present();

        void invalidate() { m_SwapchainInvalid = true; }
        bool updateSwapchain();

    private:

        window_id m_WindowID = window_id_INVALID;

        IDXGISwapChain4* m_Swapchain = nullptr;
        mutable jarray<DirectX12Texture> m_SwapchainBuffers;
        math::uvector2 m_SwapchainBuffersSize = { 0, 0 };
        uint8 m_CurrentBufferIndex = 0;
        
        bool m_SwapchainInvalid = true;


        bool init(window_id windowID);
        bool getSwapchainBuffers(uint8 buffersCount);

        void clearDirectX();
        void clearSwapchainBuffers();

        void onWindowPropertiesChanged(WindowController* windowController, const WindowData* windowData);
    };
}

#endif
