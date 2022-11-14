﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/RenderPipeline.h"

#include "../../include/JumaRE/RenderEngine.h"
#include "../../include/JumaRE/RenderOptions.h"
#include "../../include/JumaRE/RenderTarget.h"
#include "../../include/JumaRE/vertex/VertexBuffer.h"

namespace JumaRenderEngine
{
    RenderPipeline::~RenderPipeline()
    {
        clearData();
    }

    bool RenderPipeline::init()
    {
        if (!initInternal())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize render pipeline"));
            clearData();
            return false;
        }
        return true;
    }
    bool RenderPipeline::initInternal()
    {
        return true;
    }

    void RenderPipeline::clearData()
    {
        m_PipelineStages.clear();
        m_PipelineStagesQueueValid = false;
        m_PipelineStagesQueue.clear();
    }

    bool RenderPipeline::buildPipelineQueue()
    {
        if (m_PipelineStagesQueueValid)
        {
            return true;
        }
        if (m_PipelineStages.isEmpty())
        {
            return false;
        }

        m_PipelineStagesQueue.clear();
        jmap<jstringID, RenderPipelineStage> cachedPipelineStages = m_PipelineStages;
        jarray<jstringID> handledStages;
        jarray<jstringID> stagesForSync;
        while (!cachedPipelineStages.isEmpty())
        {
            // Get stages without synced dependencies
            for (const auto& stage : cachedPipelineStages)
            {
                if (stage.value.dependencies.isEmpty())
                {
                    handledStages.add(stage.key);
                }
            }
            if (handledStages.isEmpty())
            {
                JUTILS_LOG(error, JSTR("Failed validate render pipeline queue"));
                return false;
            }

            // Add them to queue
            for (const auto& stage : handledStages)
            {
                cachedPipelineStages.remove(stage);
                if (!stagesForSync.isEmpty())
                {
                    m_PipelineStagesQueue.add({ stage, stagesForSync });
                    stagesForSync.clear();
                }
                else
                {
                    m_PipelineStagesQueue.add({ stage, {} });
                }
            }

            // Remove them from dependencies and mark as needed to sync
            for (auto& stage : cachedPipelineStages)
            {
                for (const auto& handledStage : handledStages)
                {
                    stage.value.dependencies.remove(handledStage);
                }
            }
            stagesForSync = handledStages;
            handledStages.clear();
        }

        m_PipelineStagesQueueValid = true;
        return true;
    }

    bool RenderPipeline::addPipelineStage(const jstringID& stageName, RenderTarget* renderTarget)
    {
        if ((stageName == jstringID_NONE) || (renderTarget == nullptr))
        {
            JUTILS_LOG(error, JSTR("Invalid input params"));
            return false;
        }
        if (m_PipelineStages.contains(stageName))
        {
            JUTILS_LOG(warning, JSTR("Stage already exists"));
            return false;
        }

        RenderPipelineStage& newStage = m_PipelineStages.add(stageName);
        newStage.renderTarget = renderTarget;
        m_PipelineStagesQueueValid = false;

        renderTarget->OnStartDestroying.bind(this, &RenderPipeline::onRenderTargetStartDestroying);
        return true;
    }
    bool RenderPipeline::removePipelineStage(const jstringID& stageName)
    {
        const RenderPipelineStage* stage = m_PipelineStages.find(stageName);
        if (stage != nullptr)
        {
            stage->renderTarget->OnStartDestroying.unbind(this, &RenderPipeline::onRenderTargetStartDestroying);
            waitForRenderFinished();

            m_PipelineStages.remove(stageName);
            for (auto& pipelineStage : m_PipelineStages)
            {
                pipelineStage.value.dependencies.remove(stageName);
            }

            m_PipelineStagesQueueValid = false;
            return true;
        }
        return false;
    }
    void RenderPipeline::onRenderTargetStartDestroying(RenderTarget* renderTarget)
    {
        jstringID stageName = jstringID_NONE;
        for (const auto& pipelineStage : m_PipelineStages)
        {
            if (pipelineStage.value.renderTarget == renderTarget)
            {
                stageName = pipelineStage.key;
                break;
            }
        }
        if (stageName != jstringID_NONE)
        {
            removePipelineStage(stageName);
        }
    }

    bool RenderPipeline::addPipelineStageDependency(const jstringID& stageName, const jstringID& dependencyStageName)
    {
        if (dependencyStageName == jstringID_NONE)
        {
            JUTILS_LOG(error, JSTR("Invalid dependency name"));
            return false;
        }

        RenderPipelineStage* stage = m_PipelineStages.find(stageName);
        if (stage == nullptr)
        {
            JUTILS_LOG(error, JSTR("There is no stage {}"), stageName.toString());
            return false;
        }
        if (stage->dependencies.contains(dependencyStageName))
        {
            return true;
        }

        const RenderPipelineStage* dependencyStage = m_PipelineStages.find(dependencyStageName);
        if (dependencyStage == nullptr)
        {
            JUTILS_LOG(error, JSTR("There is no dependency stage {}"), dependencyStageName.toString());
            return false;
        }
        if (dependencyStage->renderTarget->isWindowRenderTarget())
        {
            JUTILS_LOG(error, JSTR("You can't set window stage as dependency"));
            return false;
        }

        stage->dependencies.add(dependencyStageName);
        m_PipelineStagesQueueValid = false;
        return true;
    }
    bool RenderPipeline::removePipelineStageDependency(const jstringID& stageName, const jstringID& dependencyStageName)
    {
        RenderPipelineStage* stage = m_PipelineStages.find(stageName);
        if ((stage != nullptr) && stage->dependencies.remove(dependencyStageName))
        {
            m_PipelineStagesQueueValid = false;
            return true;
        }
        return false;
    }

    bool RenderPipeline::render()
    {
        if (!isPipelineQueueValid())
        {
            return false;
        }
        renderInternal();
        return true;
    }
    void RenderPipeline::renderInternal()
    {
        callRender<RenderOptions>();
    }
    void RenderPipeline::callRender(RenderOptions* renderOptions)
    {
        renderOptions->renderPipeline = this;
        if (onStartRender(renderOptions))
        {
            for (const auto& renderQueueEntry : getPipelineQueue())
            {
                const RenderPipelineStage* pipelineStage = getPipelineStage(renderQueueEntry.stage);
                if (!onStartRenderToRenderTarget(renderOptions, pipelineStage->renderTarget))
                {
                    JUTILS_LOG(warning, JSTR("Failed to start render to render target {}"), renderQueueEntry.stage.toString());
                    break;
                }

                for (const auto& renderPrimitive : pipelineStage->renderTarget->getRenderPrimitives())
                {
                    renderPrimitive.vertexBuffer->render(renderOptions, renderPrimitive.material);
                }

                onFinishRenderToRenderTarget(renderOptions, pipelineStage->renderTarget);
            }

            onFinishRender(renderOptions);
        }
    }

    bool RenderPipeline::onStartRender(RenderOptions* renderOptions)
    {
        return getRenderEngine()->getWindowController()->onStartRender();
    }
    bool RenderPipeline::onStartRenderToRenderTarget(RenderOptions* renderOptions, RenderTarget* renderTarget)
    {
        renderOptions->renderTarget = renderTarget;
        return renderTarget->onStartRender(renderOptions);
    }
    void RenderPipeline::onFinishRenderToRenderTarget(RenderOptions* renderOptions, RenderTarget* renderTarget)
    {
        renderTarget->onFinishRender(renderOptions);
    }
    void RenderPipeline::onFinishRender(RenderOptions* renderOptions)
    {
        getRenderEngine()->getWindowController()->onFinishRender();
    }
}
