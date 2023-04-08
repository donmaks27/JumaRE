// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"
#include "texture/TextureBase.h"

#include "RenderPrimitivesList.h"
#include "render_target_id.h"
#include "texture/TextureFormat.h"
#include "texture/TextureSamples.h"
#include "window/window_id.h"

namespace JumaRenderEngine
{
    class RenderTarget;
    class WindowController;
    struct WindowData;
    struct RenderOptions;
    
    class RenderTarget : public TextureBase
    {
        friend RenderEngine;

        using Super = TextureBase;

    public:
        RenderTarget() : Super(RenderEngineAssetType::RenderTarget) {}
        virtual ~RenderTarget() override;

        render_target_id getID() const { return m_RenderTargetID; }

        bool isWindowRenderTarget() const { return m_WindowID != window_id_INVALID; }
        window_id getWindowID() const { return m_WindowID; }

        TextureSamples getSampleCount() const { return m_TextureSamples; }
        TextureFormat getColorFormat() const { return m_ColorFormat; }
        bool isDepthEnabled() const { return m_DepthEnabled; }

        void setSize(const math::uvector2& size);
        void setSampleCount(TextureSamples samples);
        void setColorFormat(TextureFormat format);
        void setDepthEnabled(bool enabled);

        int32 getRenderStagesCount() const { return m_RenderStages.getSize(); }
        bool isRenderStageIndexValid(const int32 renderStageIndex) const { return m_RenderStages.isValidIndex(renderStageIndex); }
        const RenderStage* getRenderStage(const int32 index) const { return m_RenderStages.findByIndex(index); }

        void setupRenderStages(const jarray<RenderStageProperties>& stages);
        bool addPrimitiveToRenderStage(int32 renderStageIndex, const RenderPrimitive& primitive);
        void clearPrimitivesList();

        virtual bool onStartRender(RenderOptions* renderOptions);
        virtual void onFinishRender(RenderOptions* renderOptions);

        void invalidate() { m_Invalid = true; }
        bool update();

    protected:

        virtual bool initInternal() { return true; }
        virtual void onClearAsset() override final;
        virtual void clearAssetInternal() { clearData(); }

        virtual bool recreateRenderTarget() { return false; }

    private:

        render_target_id m_RenderTargetID = render_target_id_INVALID;

        window_id m_WindowID = window_id_INVALID;

        TextureSamples m_TextureSamples = TextureSamples::X1;
        TextureFormat m_ColorFormat = TextureFormat::RGBA8;
        bool m_DepthEnabled = true;
        bool m_Invalid = true;
        
        jarray<RenderStage> m_RenderStages;


        bool init(render_target_id renderTargetID, window_id windowID, TextureSamples samples);
        bool init(render_target_id renderTargetID, TextureFormat format, const math::uvector2& size, TextureSamples samples);

        void clearData();

        void onWindowPropertiesChanged(WindowController* windowController, const WindowData* windowData);
    };
}
