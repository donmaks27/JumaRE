// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/RenderTarget.h"

#include "JumaRE/RenderEngine.h"

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
        m_TextureSize = windowData->size;
        if (!initInternal())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize window render target"));
            clearData();
            return false;
        }

        m_Invalid = false;
        windowController->onWindowPropertiesChanged.bind(this, &RenderTarget::onWindowPropertiesChanged);
        setupRenderStages({ {} });
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
        m_TextureSize = size;
        m_TextureSamples = samples;
        if (!initInternal())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize window render target"));
            clearData();
            return false;
        }

        m_Invalid = false;
        setupRenderStages({ {} });
        return true;
    }

    void RenderTarget::clearData()
    {
        m_RenderStages.clear();

        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->onWindowPropertiesChanged.unbind(this, &RenderTarget::onWindowPropertiesChanged);
        }

        m_WindowID = window_id_INVALID;
        m_TextureSamples = TextureSamples::X1;
        m_TextureSize = { 0, 0 };
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
            if ((windowData->size != m_TextureSize) || (windowData->samples != m_TextureSamples))
            {
                m_TextureSize = windowData->size;
                m_TextureSamples = windowData->samples;
                invalidate();
            }
        }
    }

    void RenderTarget::setSize(const math::uvector2& size)
    {
        if (!isWindowRenderTarget() && (m_TextureSize != size))
        {
            m_TextureSize = size;
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

    void RenderTarget::setupRenderStages(const jarray<RenderStageProperties>& stages)
    {
        m_RenderStages.resize(stages.getSize());
        for (int32 index = 0; index < m_RenderStages.getSize(); index++)
        {
	        m_RenderStages[index].properties = stages[index];
	        m_RenderStages[index].primitivesList.clear();
        }
    }
    bool RenderTarget::addPrimitiveToRenderStage(const int32 renderStageIndex, const RenderPrimitive& primitive)
    {
        if (!m_RenderStages.isValidIndex(renderStageIndex))
        {
	        JUTILS_LOG(warning, JSTR("Invalid render stage index {}"), renderStageIndex);
            return false;
        }
        if ((primitive.vertexBuffer == nullptr) || (primitive.material == nullptr))
        {
            JUTILS_LOG(warning, JSTR("Invalid primitive"));
            return false;
        }
        m_RenderStages[renderStageIndex].primitivesList.add(primitive);
        return true;
    }
    void RenderTarget::clearPrimitivesList()
    {
        for (auto& stage : m_RenderStages)
        {
	        stage.primitivesList.clear();
        }
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
