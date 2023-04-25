// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/RenderEngine.h"

#include "JumaRE/RenderPipeline.h"
#include "JumaRE/RenderTarget.h"
#include "JumaRE/material/Material.h"
#include "JumaRE/material/Shader.h"
#include "JumaRE/texture/Texture.h"
#include "JumaRE/vertex/VertexBuffer.h"
#include "JumaRE/vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    RenderEngine* RenderEngineContextObjectBase::s_RenderEngine = nullptr;

    RenderEngine::~RenderEngine()
    {
        clearData();
        RenderEngineContextObjectBase::s_RenderEngine = nullptr;
    }

    bool RenderEngine::init(const RenderEngineCreateInfo& createInfo)
    {
        if (isValid())
        {
            JUTILS_LOG(warning, JSTR("Render engine already initialized"));
            return false;
        }

        RenderEngineContextObjectBase::s_RenderEngine = this;

        WindowController* windowController = createWindowController();
        if (!windowController->initWindowController())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize window controller"));
            delete windowController;
            RenderEngineContextObjectBase::s_RenderEngine = nullptr;
            return false;
        }
        m_WindowController = windowController;
        if (!initInternal(createInfo.mainWindowInfo))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize render engine"));
            clearInternal();
            RenderEngineContextObjectBase::s_RenderEngine = nullptr;
            return false;
        }
        m_Initialized = true;

        if (!createRenderAssets())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize render assets"));
            clear();
            return false;
        }
        if (!m_AsyncAssetTaskQueue.init(math::max(1, createInfo.assetTaskWorkerCount), this))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize assets loading task queue"));
            clear();
            return false;
        }
        return true;
    }
    bool RenderEngine::initInternal(const WindowCreateInfo& mainWindowInfo)
    {
        return m_WindowController->createMainWindow(mainWindowInfo);
    }

    bool RenderEngine::createRenderAssets()
    {
        RenderPipeline* renderPipeline = createRenderPipelineInternal();
        if (!renderPipeline->init())
        {
            JUTILS_LOG(error, JSTR("Failed to init render pipeline"));
            delete renderPipeline;
            return false;
        }
        m_RenderPipeline = renderPipeline;

        if (!m_WindowController->createRenderTargets())
        {
            JUTILS_LOG(error, JSTR("Failed to create render targets for windows"));
            return false;
        }

        constexpr uint8 defaultTextureData[] = { 255, 0, 255, 255 };
        m_DefaultTexture = createTexture({ 1, 1 }, TextureFormat::RGBA8, defaultTextureData);
        if (m_DefaultTexture == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create default texture"));
            return false;
        }

        return true;
    }
    void RenderEngine::clearAssets()
    {
        m_DefaultTexture = nullptr;
        if (m_RenderPipeline != nullptr)
        {
            delete m_RenderPipeline;
            m_RenderPipeline = nullptr;
        }
        if (m_WindowController != nullptr)
        {
            m_WindowController->destroyRenderTargets();
        }
    }

    void RenderEngine::clear()
    {
        if (m_Initialized)
        {
            onDestroying.call(this);

            m_AsyncAssetTaskQueue.stop();
            for (const auto& task : m_RenderAssets_DestroyTasks)
            {
                if (!task.m_TaskFinished)
                {
                    task.m_Asset->clearAsset();
                }
            }
            for (const auto& asset : m_RenderAssets_MarkedForDestroy)
            {
                if (asset.first != nullptr)
                {
                    asset.first->clearAsset();
                }
            }
            m_RenderAssets_DestroyTasks.clear();
            m_RenderAssets_MarkedForDestroy.clear();

            clearInternal();

            m_Initialized = false;
            RenderEngineContextObjectBase::s_RenderEngine = nullptr;
        }
    }
    void RenderEngine::clearData()
    {
        if (m_WindowController != nullptr)
        {
            delete m_WindowController;
            m_WindowController = nullptr;
        }
        m_RegisteredVertexComponents.clear();
        m_RegisteredVertices.clear();
        m_RegisteredVerticesData.clear();
        m_VertexIDGenerator.reset();
    }
    
    RenderPipeline* RenderEngine::createRenderPipelineInternal()
    {
        return createObject<RenderPipeline>();
    }

    bool RenderEngine::createShaderAsync(const ShaderCreateInfo& createInfo, const std::function<void(Shader*)>& callback)
    {
        Shader* shader = allocateShader();
        if (shader == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to allocate shader"));
	        return false;
        }
        const bool taskStarted = m_AsyncAssetTaskQueue.addTask(new jasync_task_default([this, shader, createInfo = createInfo, callback = callback]()
        {
	        if (!shader->init(createInfo))
	        {
                destroyAsset(shader);
	        }
            else
            {
	            callback(shader);
            }
        }));
        if (!taskStarted)
        {
            JUTILS_LOG(error, JSTR("Failed to start async shader creation"));
	        deallocateShader(shader);
            return false;
        }
        return true;
    }
    bool RenderEngine::createShaderAsync(const ShaderCreateInfo& createInfo, OnAssetCreatedTask<Shader>* onAssetCreated)
    {
        if (onAssetCreated == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Empty callback task"));
            return false;
        }
        Shader* shader = allocateShader();
        if (shader == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to allocate shader"));
	        return false;
        }
        const bool taskStarted = m_AsyncAssetTaskQueue.addTask(new AsyncAssetCreateTask([this, shader, createInfo = createInfo, onAssetCreated]()
        {
	        if (!shader->init(createInfo))
	        {
                destroyAsset(shader);
	        }
            else
            {
                onAssetCreated->m_Asset = shader;
            }
        }, onAssetCreated));
        if (!taskStarted)
        {
            JUTILS_LOG(error, JSTR("Failed to start async shader creation"));
	        deallocateShader(shader);
            return false;
        }
        return true;
    }
    Shader* RenderEngine::createShader(const ShaderCreateInfo& createInfo)
    {
        Shader* shader = allocateShader();
        if (!shader->init(createInfo))
        {
            deallocateShader(shader);
            return nullptr;
        }
        return shader;
    }
    void RenderEngine::destroyShader(Shader* shader)
    {
        if (shader != nullptr)
        {
            shader->clearAsset();
            deallocateShader(shader);
        }
    }

    Material* RenderEngine::createMaterial(Shader* shader)
    {
        if (shader == nullptr)
        {
            return nullptr;
        }
        Material* material = allocateMaterial();
        ++shader->m_ChildMaterialsCount;
        if (!material->init(shader))
        {
            --shader->m_ChildMaterialsCount;
            deallocateMaterial(material);
            return nullptr;
        }
        return material;
    }
    void RenderEngine::destroyMaterial(Material* material)
    {
        if (material != nullptr)
        {
            material->clearAsset();
            deallocateMaterial(material);
        }
    }
    
    void RenderEngine::registerVertexComponent(const jstringID& vertexComponentID, const VertexComponentDescription& description)
    {
        if (vertexComponentID != jstringID_NONE)
        {
            m_RegisteredVertexComponents.add(vertexComponentID, description);
        }
    }
    vertex_id RenderEngine::registerVertex(const VertexDescription& description)
    {
        if (description.components.isEmpty())
        {
            return vertex_id_NONE;
        }
        const vertex_id* vertexIDPtr = m_RegisteredVertices.find(description);
        if (vertexIDPtr != nullptr)
        {
            return *vertexIDPtr;
        }

        uint32 vertexSize = 0;
        for (const auto& componentID : description.components)
        {
            const VertexComponentDescription* componentDescription = findVertexComponent(componentID);
            if (componentDescription == nullptr)
            {
                JUTILS_LOG(error, JSTR("Invalid vertex component {}"), componentID.toString());
                return vertex_id_NONE;
            }
            vertexSize += GetVertexComponentSize(componentDescription->type);
        }

        const vertex_id vertexID = m_VertexIDGenerator.getUID();
        if (vertexID == vertex_id_NONE)
        {
            return vertex_id_NONE;
        }
        m_VertexIDGenerator.generateUID();
        onRegisteredVertex(vertexID, m_RegisteredVerticesData.add(vertexID, { description, vertexSize }));
        return vertexID;
    }

    VertexBuffer* RenderEngine::createVertexBuffer(const VertexBufferData& data)
    {
        const vertex_id vertexID = registerVertex(data.vertexDescription);
        if (vertexID == vertex_id_NONE)
        {
            return nullptr;
        }

        VertexBuffer* vertexBuffer = allocateVertexBuffer();
        if (!vertexBuffer->init(vertexID, data))
        {
            destroyVertexBuffer(vertexBuffer);
            return nullptr;
        }
        return vertexBuffer;
    }
    void RenderEngine::destroyVertexBuffer(VertexBuffer* vertexBuffer)
    {
        if (vertexBuffer != nullptr)
        {
            vertexBuffer->clearAsset();
            deallocateVertexBuffer(vertexBuffer);
        }
    }

    Texture* RenderEngine::createTexture(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        Texture* texture = allocateTexture();
        if (!texture->init(size, format, data))
        {
            destroyTexture(texture);
            return nullptr;
        }
        return texture;
    }
    void RenderEngine::destroyTexture(Texture* texture)
    {
        if (texture != nullptr)
        {
            texture->clearAsset();
            deallocateTexture(texture);
        }
    }
    
    RenderTarget* RenderEngine::getRenderTarget(const render_target_id renderTargetID) const
    {
        RenderTarget* const* renderTarget = m_RenderTargets.find(renderTargetID);
        return renderTarget != nullptr ? *renderTarget : nullptr;
    }
    RenderTarget* RenderEngine::createWindowRenderTarget(const window_id windowID, const TextureSamples samples)
    {
        RenderTarget* renderTarget = allocateRenderTarget();
        const render_target_id renderTargetID = m_RenderTagetIDs.getUID();
        if ((renderTargetID == render_target_id_INVALID) || !renderTarget->init(renderTargetID, windowID, samples))
        {
            destroyRenderTarget(renderTarget);
            return nullptr;
        }
        m_RenderTagetIDs.generateUID();
        m_RenderTargets.add(renderTargetID, renderTarget);
        m_RenderPipeline->onRenderTargetCreated(renderTarget);
        return renderTarget;
    }
    RenderTarget* RenderEngine::createRenderTarget(const TextureFormat format, const math::uvector2& size, const TextureSamples samples)
    {
        RenderTarget* renderTarget = allocateRenderTarget();
        const render_target_id renderTargetID = m_RenderTagetIDs.getUID();
        if ((renderTargetID == render_target_id_INVALID) || !renderTarget->init(renderTargetID, format, size, samples))
        {
            destroyRenderTarget(renderTarget);
            return nullptr;
        }
        m_RenderTagetIDs.generateUID();
        m_RenderTargets.add(renderTargetID, renderTarget);
        m_RenderPipeline->onRenderTargetCreated(renderTarget);
        return renderTarget;
    }
    void RenderEngine::destroyRenderTarget(RenderTarget* renderTarget)
    {
        if (renderTarget != nullptr)
        {
            renderTarget->clearAsset();
            deallocateRenderTarget(renderTarget);
        }
    }

    void RenderEngine::destroyAsset(RenderEngineAsset* asset)
    {
        if (asset == nullptr)
        {
	        return;
        }

        // TODO: Get amount of rendered frames
        constexpr uint8 framesDelay = 1;
        m_RenderAssets_MarkedForDestroyMutex.lock();
        m_RenderAssets_MarkedForDestroy.add({ asset, framesDelay });
        m_RenderAssets_MarkedForDestroyMutex.unlock();
    }
    
    bool RenderEngine::render()
    {
        if (!m_RenderPipeline->buildRenderTargetsQueue())
        {
            JUTILS_LOG(error, JSTR("Failed to build render targets queue"));
            return false;
        }

        if (!m_RenderPipeline->render())
        {
            JUTILS_LOG(error, JSTR("Render failed"));
            return false;
        }
        m_WindowController->updateWindows();
        for (const auto& renderTarget : m_RenderTargets.values())
        {
            renderTarget->clearPrimitivesList();
        }

        processMarkedForDestroyAssets();
        processFinishedDestroyAssetTasks();
        return true;
    }

    void RenderEngine::processMarkedForDestroyAssets()
    {
        m_RenderAssets_MarkedForDestroyMutex.lock();
        auto iter = m_RenderAssets_MarkedForDestroy.begin();
        while (iter.isValid())
        {
            if (iter->second == 0)
            {
                RenderEngineAsset* asset = iter->first;
                if (asset->isReadyForDestroy())
                {
                    m_RenderAssets_DestroyTasksTemp.add(&m_RenderAssets_DestroyTasks.put(asset));
                    m_RenderAssets_MarkedForDestroy.removeAt(iter);
                }
            }
            else
            {
                iter->second--;
                ++iter;
            }
        }
        m_RenderAssets_MarkedForDestroyMutex.unlock();
        if (!m_RenderAssets_DestroyTasksTemp.isEmpty())
        {
            m_AsyncAssetTaskQueue.addTasks(m_RenderAssets_DestroyTasksTemp);
            m_RenderAssets_DestroyTasksTemp.clear();
        }
    }
    void RenderEngine::processFinishedDestroyAssetTasks()
    {
        m_RenderAssets_DestroyTasks.removeByPredicate([this](const AsyncAssetDestroyTask& task) -> bool
        {
            if (!task.m_TaskFinished)
            {
                return false;
            }
            if (task.m_Asset != nullptr)
            {
                switch (task.m_Asset->getType())
                {
                case RenderEngineAssetType::Shader:       deallocateShader(dynamic_cast<Shader*>(task.m_Asset)); break;
                case RenderEngineAssetType::Material:     deallocateMaterial(dynamic_cast<Material*>(task.m_Asset)); break;
                case RenderEngineAssetType::Texture:      deallocateTexture(dynamic_cast<Texture*>(task.m_Asset)); break;
                case RenderEngineAssetType::RenderTarget: deallocateRenderTarget(dynamic_cast<RenderTarget*>(task.m_Asset)); break;
                case RenderEngineAssetType::VertexBuffer: deallocateVertexBuffer(dynamic_cast<VertexBuffer*>(task.m_Asset)); break;
                default: ;
                }
            }
            return true;
        });
    }
    void RenderEngine::AsyncAssetDestroyTask::run()
    {
        if (m_Asset != nullptr)
        {
            m_Asset->clearAsset();
        }
        m_TaskFinished = true;
    }
}
