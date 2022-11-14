﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"
#include "RenderEngineContextObject.h"

#include <jutils/jmap.h>
#include <jutils/jset.h>
#include <jutils/jstringID.h>

namespace JumaRenderEngine
{
    struct RenderOptions;
    class RenderTarget;

    struct RenderPipelineStage
    {
        RenderTarget* renderTarget = nullptr;
        jset<jstringID> dependencies;
    };
    struct RenderPipelineStageQueueEntry
    {
        jstringID stage = jstringID_NONE;
        jarray<jstringID> stagesForSynchronization;
    };

    class RenderPipeline : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        RenderPipeline() = default;
        virtual ~RenderPipeline() override;

        bool isPipelineQueueValid() const { return m_PipelineStagesQueueValid; }
        const jarray<RenderPipelineStageQueueEntry>& getPipelineQueue() const { return m_PipelineStagesQueue; }
        bool buildPipelineQueue();

        const jmap<jstringID, RenderPipelineStage>& getPipelineStages() const { return m_PipelineStages; }
        const RenderPipelineStage* getPipelineStage(const jstringID& stageName) const { return m_PipelineStages.find(stageName); }

        bool addPipelineStage(const jstringID& stageName, RenderTarget* renderTarget);
        bool removePipelineStage(const jstringID& stageName);
        bool addPipelineStageDependency(const jstringID& stageName, const jstringID& dependencyStageName);
        bool removePipelineStageDependency(const jstringID& stageName, const jstringID& dependencyStageName);

        bool render();
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

        jmap<jstringID, RenderPipelineStage> m_PipelineStages;

        bool m_PipelineStagesQueueValid = false;
        jarray<RenderPipelineStageQueueEntry> m_PipelineStagesQueue;


        bool init();

        void onRenderTargetStartDestroying(RenderTarget* renderTarget);

        void clearData();
        
        void callRender(RenderOptions* renderOptions);
    };
}
