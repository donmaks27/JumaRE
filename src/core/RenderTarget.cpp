// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/RenderTarget.h"

#include "../../include/JumaRE/RenderEngine.h"

namespace JumaRenderEngine
{
    RenderTarget::~RenderTarget()
    {
        clearData();
    }

    bool RenderTarget::init(const render_target_id renderTargetID, const window_id windowID, const TextureSamples samples)
    {
        WindowController* windowController = getRenderEngine()->getWindowController();
        const WindowData* windowData = windowController->findWindowData(windowID);
        if (windowData == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to find window {}"), windowID);
            return false;
        }

        m_RenderTargetID = renderTargetID;
        m_WindowID = windowID;
        m_TextureSamples = samples;
        m_Size = windowData->size;
        if (!initInternal())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize window render target"));
            clearData();
            return false;
        }

        m_Invalid = false;
        windowController->OnWindowPropertiesChanged.bind(this, &RenderTarget::onWindowPropertiesChanged);
        return true;
    }
    bool RenderTarget::init(const render_target_id renderTargetID, const TextureFormat format, const math::uvector2& size, const TextureSamples samples)
    {
        if ((size.x == 0) || (size.y == 0))
        {
            JUTILS_LOG(error, JSTR("Invalid input params"));
            return false;
        }

        m_RenderTargetID = renderTargetID;
        m_ColorFormat = format;
        m_Size = size;
        m_TextureSamples = samples;
        if (!initInternal())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize window render target"));
            clearData();
            return false;
        }

        m_Invalid = false;
        return true;
    }

    void RenderTarget::clearData()
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->OnWindowPropertiesChanged.unbind(this, &RenderTarget::onWindowPropertiesChanged);
        }

        m_WindowID = window_id_INVALID;
        m_TextureSamples = TextureSamples::X1;
        m_Size = { 0, 0 };
        m_ColorFormat = TextureFormat::RGBA8;
    }

    bool RenderTarget::update()
    {
        if (m_Invalid)
        {
            if (!recreateRenderTarget())
            {
                JUTILS_LOG(error, JSTR("Failed to recreate render target"));
                return false;
            }
            m_Invalid = false;
        }
        return true;
    }

    void RenderTarget::clearAsset()
    {
        OnStartDestroying.call(this);
        clearAssetInternal();
    }

    void RenderTarget::onWindowPropertiesChanged(WindowController* windowController, const WindowData* windowData)
    {
        if (windowData->windowID == getWindowID())
        {
            if ((windowData->size != m_Size) || (windowData->samples != m_TextureSamples))
            {
                m_Size = windowData->size;
                m_TextureSamples = windowData->samples;
                invalidate();
            }
        }
    }

    void RenderTarget::setSize(const math::uvector2& size)
    {
        if (!isWindowRenderTarget() && (m_Size != size))
        {
            m_Size = size;
            invalidate();
        }
    }
    void RenderTarget::setSampleCount(const TextureSamples samples)
    {
        if (m_TextureSamples != samples)
        {
            m_TextureSamples = samples;
            invalidate();
        }
    }
    void RenderTarget::setColorFormat(const TextureFormat format)
    {
        if (m_ColorFormat != format)
        {
            m_ColorFormat = format;
            invalidate();
        }
    }
    void RenderTarget::setDepthEnabled(const bool enabled)
    {
        if (m_DepthEnabled != enabled)
        {
            m_DepthEnabled = enabled;
            invalidate();
        }
    }

    bool RenderTarget::addRenderPrimitive(const RenderTargetPrimitive& primitive)
    {
        if ((primitive.vertexBuffer == nullptr) || (primitive.material == nullptr))
        {
            return false;
        }
        m_RenderPrimitives.add(primitive);
        return true;
    }
    void RenderTarget::clearRenderPrimitives()
    {
        m_RenderPrimitives.clear();
    }

    bool RenderTarget::onStartRender(RenderOptions* renderOptions)
    {
        if (!update())
        {
            return false;
        }
        if (isWindowRenderTarget())
        {
            return getRenderEngine()->getWindowController()->onStartWindowRender(getWindowID());
        }
        return true;
    }
    void RenderTarget::onFinishRender(RenderOptions* renderOptions)
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->onFinishWindowRender(getWindowID());
        }
    }
}
