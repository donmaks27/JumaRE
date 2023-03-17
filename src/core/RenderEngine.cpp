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
    RenderEngine::~RenderEngine()
    {
        clearData();
    }

    bool RenderEngine::init(const RenderEngineCreateInfo& createInfo)
    {
        if (isValid())
        {
            JUTILS_LOG(warning, JSTR("Render engine already initialized"));
            return false;
        }

        WindowController* windowController = createWindowController();
        if (!windowController->initWindowController())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize window controller"));
            delete windowController;
            return false;
        }
        m_WindowController = windowController;
        if (!initInternal(createInfo.mainWindowInfo))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize render engine"));
            clearInternal();
            return false;
        }
        m_Initialized = true;

        if (!createRenderAssets())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize render assets"));
            clear();
            return false;
        }
        const bool success = m_AssetLoadingTaskQueue.start(math::max(1, createInfo.assetsLoadingWorkers), [this](const int32 workerIndex){ 
            return initAssetLoadingWorker(workerIndex); 
        }, [this](const int32 workerIndex){
            clearAssetLoadingWorker(workerIndex);
        });
        if (!success)
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

            m_AssetLoadingTaskQueue.stop();
            clearInternal();

            m_Initialized = false;
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

    void RenderEngine::registerObjectInternal(RenderEngineContextObjectBase* object)
    {
        if (object != nullptr)
        {
            object->m_RenderEngine = this;
        }
    }

    RenderPipeline* RenderEngine::createRenderPipelineInternal()
    {
        return createObject<RenderPipeline>();
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

    Shader* RenderEngine::createShaderSync(const ShaderCreateInfo& createInfo)
    {
        Shader* shader = allocateShader();
        if (!shader->init(createInfo.fileNames, createInfo.vertexComponents, createInfo.uniforms))
        {
            destroyShader(shader);
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

    Material* RenderEngine::createMaterialSync(Shader* shader)
    {
        Material* material = allocateMaterial();
        if (!material->init(shader))
        {
            destroyMaterial(material);
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

    bool RenderEngine::createShader(const ShaderCreateInfo& createInfo, std::function<void(Shader*)> callback)
    {
        return callback && m_AssetLoadingTaskQueue.addTask([this, createInfo, resultCallback = std::move(callback)](){
            resultCallback(createShaderSync(createInfo));
        });
    }
    bool RenderEngine::createMaterial(Shader* shader, std::function<void(Material*)> callback)
    {
        return callback && m_AssetLoadingTaskQueue.addTask([this, shader, resultCallback = std::move(callback)](){
            resultCallback(createMaterialSync(shader));
        });
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
        for (const auto& renderTarget : m_RenderTargets)
        {
            renderTarget.value->clearPrimitivesList();
        }
        m_WindowController->updateWindows();
        return true;
    }
}
