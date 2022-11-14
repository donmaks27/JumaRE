// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12)

#include "RenderPipeline_DirectX12.h"

#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "DirectX12Objects/DirectX12CommandList.h"
#include "DirectX12Objects/DirectX12Swapchain.h"
#include "window/WindowController_DirectX12.h"

namespace JumaRenderEngine
{
    RenderPipeline_DirectX12::~RenderPipeline_DirectX12()
    {
        clearDirectX();
    }

    void RenderPipeline_DirectX12::clearDirectX()
    {
        waitForPreviousRenderFinish();
    }

    void RenderPipeline_DirectX12::renderInternal()
    {
        callRender<RenderOptions_DirectX12>();
    }

    bool RenderPipeline_DirectX12::onStartRender(RenderOptions* renderOptions)
    {
        if (!Super::onStartRender(renderOptions))
        {
            return false;
        }

        waitForPreviousRenderFinish();

        const WindowController* windowController = getRenderEngine()->getWindowController();
        for (const auto& windowID : windowController->getWindowIDs())
        {
            const WindowData_DirectX12* windowData = windowController->findWindowData<WindowData_DirectX12>(windowID);
            DirectX12Swapchain* swapchain = windowData != nullptr ? windowData->swapchain : nullptr;
            if (swapchain == nullptr)
            {
                JUTILS_LOG(warning, JSTR("Failed to get swapchain for window {}"), windowID);
                return false;
            }
            if (!swapchain->updateSwapchain())
            {
                JUTILS_LOG(error, JSTR("Failed to update swapchain for window {}"), windowID);
                return false;
            }
        }

        return startCommandListRecord(renderOptions);
    }
    bool RenderPipeline_DirectX12::onStartRenderToRenderTarget(RenderOptions* renderOptions, RenderTarget* renderTarget)
    {
        return Super::onStartRenderToRenderTarget(renderOptions, renderTarget);
    }
    void RenderPipeline_DirectX12::onFinishRenderToRenderTarget(RenderOptions* renderOptions, RenderTarget* renderTarget)
    {
        Super::onFinishRenderToRenderTarget(renderOptions, renderTarget);
    }
    void RenderPipeline_DirectX12::onFinishRender(RenderOptions* renderOptions)
    {
        finishCommandListRecord(renderOptions);
        Super::onFinishRender(renderOptions);
    }

    void RenderPipeline_DirectX12::waitForRenderFinished()
    {
        waitForPreviousRenderFinish();
        Super::waitForRenderFinished();
    }
    void RenderPipeline_DirectX12::waitForPreviousRenderFinish()
    {
        if (m_RenderCommandList != nullptr)
        {
            m_RenderCommandList->waitForFinish();
            m_RenderCommandList->markUnused();
            m_RenderCommandList = nullptr;
        }
    }
    bool RenderPipeline_DirectX12::startCommandListRecord(RenderOptions* renderOptions)
    {
        DirectX12CommandQueue* commandQueue = getRenderEngine<RenderEngine_DirectX12>()->getCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        if (commandQueue == nullptr)
        {
            return false;
        }
        m_RenderCommandList = commandQueue->getCommandList();
        reinterpret_cast<RenderOptions_DirectX12*>(renderOptions)->renderCommandList = m_RenderCommandList;
        return true;
    }
    void RenderPipeline_DirectX12::finishCommandListRecord(RenderOptions* renderOptions)
    {
        m_RenderCommandList->execute(false);

        const WindowController* windowController = getRenderEngine()->getWindowController();
        for (const auto& windowID : windowController->getWindowIDs())
        {
            const WindowData_DirectX12* windowData = windowController->findWindowData<WindowData_DirectX12>(windowID);
            if ((windowData == nullptr) || (windowData->swapchain == nullptr))
            {
                continue;
            }
            windowData->swapchain->present();
        }

        m_RenderCommandList->signal();
    }
}

#endif
