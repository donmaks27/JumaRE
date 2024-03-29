﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/RenderPipeline.h"

#include "JumaRE/RenderEngine.h"
#include "JumaRE/RenderOptions.h"
#include "JumaRE/RenderTarget.h"
#include "JumaRE/material/Material.h"
#include "JumaRE/vertex/VertexBuffer.h"

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
        m_RenderTargetsQueueValid = false;
        m_RenderTargetsDependecies.clear();
        m_RenderTargetsQueue.clear();
    }

    void RenderPipeline::onRenderTargetCreated(RenderTarget* renderTarget)
    {
        renderTarget->onDestroying.bind(this, &RenderPipeline::onRenderTargetDestroying);

        m_RenderTargetsQueueValid = false;
        m_RenderTargetsDependecies.add(renderTarget->getID());
    }
    void RenderPipeline::onRenderTargetDestroying(RenderEngineAsset* renderTargetAsset)
    {
        renderTargetAsset->onDestroying.unbind(this, &RenderPipeline::onRenderTargetDestroying);
        const RenderTarget* renderTarget = dynamic_cast<RenderTarget*>(renderTargetAsset);

        m_RenderTargetsQueueValid = false;

        const render_target_id renderTargetID = renderTarget->getID();
        m_RenderTargetsDependecies.remove(renderTargetID);
        for (auto& renderTargetDependecies : m_RenderTargetsDependecies.values())
        {
            renderTargetDependecies.remove(renderTargetID);
        }
    }

    bool RenderPipeline::addRenderTargetDependecy(const render_target_id renderTargetID, const render_target_id dependencyRenderTargetID)
    {
        if (!m_RenderTargetsDependecies.contains(dependencyRenderTargetID))
        {
            return false;
        }
        if (getRenderEngine()->getRenderTarget(dependencyRenderTargetID)->isWindowRenderTarget())
        {
            JUTILS_LOG(error, JSTR("Window's render target can't be dependecy"));
            return false;
        }

        jset<render_target_id>* dependencies = m_RenderTargetsDependecies.find(renderTargetID);
        if (dependencies == nullptr)
        {
            return false;
        }
        if (dependencies->contains(dependencyRenderTargetID))
        {
            return true;
        }

        m_RenderTargetsQueueValid = false;
        dependencies->add(dependencyRenderTargetID);
        return true;
    }
    bool RenderPipeline::removeRenderTargetDependecy(const render_target_id renderTargetID, const render_target_id dependencyRenderTargetID)
    {
        jset<render_target_id>* dependencies = m_RenderTargetsDependecies.find(renderTargetID);
        if (dependencies == nullptr)
        {
            return false;
        }
        if (!dependencies->contains(dependencyRenderTargetID))
        {
            return true;
        }

        m_RenderTargetsQueueValid = false;
        dependencies->remove(dependencyRenderTargetID);
        return true;
    }

    bool RenderPipeline::buildRenderTargetsQueue()
    {
        if (m_RenderTargetsQueueValid)
        {
            return true;
        }
        if (m_RenderTargetsDependecies.isEmpty())
        {
            return true;
        }

        m_RenderTargetsQueue.clear();
        jmap<render_target_id, jset<render_target_id>> cachedDependencies = m_RenderTargetsDependecies;
        jarray<render_target_id> handledStages;
        jarray<render_target_id> stagesForSync;
        while (!cachedDependencies.isEmpty())
        {
            // Get stages without synced dependencies
            for (const auto& [stage, dependencies] : cachedDependencies)
            {
                if (dependencies.isEmpty())
                {
                    handledStages.add(stage);
                }
            }
            if (handledStages.isEmpty())
            {
                JUTILS_LOG(warning, JSTR("Failed validate render targets queue"));
                //return false;
                for (const auto& stage : cachedDependencies.keys())
                {
                    handledStages.add(stage);
                }
            }

            // Add them to queue
            for (const auto& stage : handledStages)
            {
                cachedDependencies.remove(stage);
                if (!stagesForSync.isEmpty())
                {
                    m_RenderTargetsQueue.add({ stage, stagesForSync });
                    stagesForSync.clear();
                }
                else
                {
                    m_RenderTargetsQueue.add({ stage, {} });
                }
            }

            // Remove them from dependencies and mark as needed to sync
            for (auto& dependencies : cachedDependencies.values())
            {
                for (const auto& handledStage : handledStages)
                {
                    dependencies.remove(handledStage);
                }
            }
            stagesForSync = handledStages;
            handledStages.clear();
        }

        m_RenderTargetsQueueValid = true;
        return true;
    }

    bool RenderPipeline::render()
    {
        if (!isRenderTargetsQueueValid())
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
            const RenderEngine* renderEngine = getRenderEngine();
            for (const auto& renderQueueEntry : m_RenderTargetsQueue)
            {
                RenderTarget* renderTarget = renderEngine->getRenderTarget(renderQueueEntry.renderTargetID);
                if (!onStartRenderToRenderTarget(renderOptions, renderTarget))
                {
                    JUTILS_LOG(warning, JSTR("Failed to start render to render target {}"), renderQueueEntry.renderTargetID);
                    break;
                }

                const int32 stagesCount = renderTarget->getRenderStagesCount();
                for (int32 index = 0; index < stagesCount; index++)
                {
	                const RenderStage* renderStage = renderTarget->getRenderStage(index);
                    if (renderStage != nullptr)
                    {
                        renderOptions->renderStageProperties = renderStage->properties;
	                    for (const auto& renderPrimitive : renderStage->primitivesList)
	                    {
		                    if (renderPrimitive.material != nullptr)
		                    {
		                        renderPrimitive.vertexBuffer->render(renderOptions, renderPrimitive.material);
		                    }
	                    }
                    }
                }

                onFinishRenderToRenderTarget(renderOptions, renderTarget);
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
