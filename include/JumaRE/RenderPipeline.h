// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"
#include "RenderEngineContextObject.h"

#include <jutils/jmap.h>
#include <jutils/jset.h>
#include <jutils/jstringID.h>

#include "render_target_id.h"

namespace JumaRenderEngine
{
    struct RenderOptions;
    class RenderTarget;

    class RenderPipeline : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        RenderPipeline() = default;
        virtual ~RenderPipeline() override;

        bool addRenderTargetDependecy(render_target_id renderTargetID, render_target_id dependencyRenderTargetID);
        bool removeRenderTargetDependecy(render_target_id renderTargetID, render_target_id dependencyRenderTargetID);

        bool isRenderTargetsQueueValid() const { return m_RenderTargetsQueueValid; }
        bool buildRenderTargetsQueue();

        virtual void waitForRenderFinished() {}

    protected:

        virtual bool initInternal();

        virtual void renderInternal();
        template<typename T, TEMPLATE_ENABLE(is_base<RenderOptions, T>)>
        void callRender()
        {
            T renderOptions;
            this->callRender(&renderOptions);
        }

        virtual bool onStartRender(RenderOptions* renderOptions);
        virtual bool onStartRenderToRenderTarget(RenderOptions* renderOptions, RenderTarget* renderTarget);
        virtual void onFinishRenderToRenderTarget(RenderOptions* renderOptions, RenderTarget* renderTarget);
        virtual void onFinishRender(RenderOptions* renderOptions);

    private:

        struct RenderTargetsQueueEntry
        {
            render_target_id renderTargetID = render_target_id_INVALID;
            jarray<render_target_id> syncRenderTargets;
        };

        jmap<render_target_id, jset<render_target_id>> m_RenderTargetsDependecies;
        jarray<RenderTargetsQueueEntry> m_RenderTargetsQueue;
        bool m_RenderTargetsQueueValid = false;


        bool init();

        void onRenderTargetCreated(RenderTarget* renderTarget);
        void onRenderTargetStartDestroying(RenderTarget* renderTarget);

        void clearData();
        
        bool render();
        void callRender(RenderOptions* renderOptions);
    };
}
