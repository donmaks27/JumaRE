// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"

#include <jutils/jtask_queue.h>

#include "RenderAPI.h"
#include "RenderPrimitivesList.h"
#include "render_target_id.h"
#include "material/ShaderUniform.h"
#include "texture/TextureFormat.h"
#include "vertex/VertexBufferData.h"
#include "window/WindowController.h"

namespace JumaRenderEngine
{
    class Material;
    class RenderEngine;
    class RenderPipeline;
    class RenderTarget;
    class Shader;
    class Texture;
    class VertexBuffer;

    struct RegisteredVertexDescription
    {
        VertexDescription description;
        uint32 vertexSize = 0;
    };

    struct RenderEngineCreateInfo
    {
        WindowCreateInfo mainWindowInfo;
        int32 assetsLoadingWorkers = 1;
    };

    JUTILS_CREATE_MULTICAST_DELEGATE1(OnRenderEngineEvent, RenderEngine*, renderEngine);

    class RenderEngine
    {
        friend WindowController;

    public:
        RenderEngine() = default;
        virtual ~RenderEngine();

        struct ShaderCreateInfo
        {
            jmap<ShaderStageFlags, jstring> fileNames;
            jset<jstringID> vertexComponents;
            jmap<jstringID, ShaderUniform> uniforms;
        };

        OnRenderEngineEvent onDestroying;


        virtual RenderAPI getRenderAPI() const = 0;

        bool init(const RenderEngineCreateInfo& createInfo);
        bool isValid() const { return m_Initialized; }
        void clear();

        WindowController* getWindowController() const { return m_WindowController; }
        template<typename T, TEMPLATE_ENABLE(is_base<WindowController, T>)>
        T* getWindowController() const { return dynamic_cast<T*>(this->getWindowController()); }

        template<typename T, TEMPLATE_ENABLE(is_base_and_not_abstract<RenderEngineContextObjectBase, T>)>
        T* createObject() { return this->registerObject(new T()); }
        template<typename T, TEMPLATE_ENABLE(is_base<RenderEngineContextObjectBase, T>)>
        T* registerObject(T* object)
        {
            this->registerObjectInternal(object);
            return object;
        }

        RenderPipeline* getRenderPipeline() const { return m_RenderPipeline; }
        RenderTarget* getRenderTarget(render_target_id renderTargetID) const;

        RenderTarget* createRenderTarget(TextureFormat format, const math::uvector2& size, TextureSamples samples);
        VertexBuffer* createVertexBuffer(const VertexBufferData& data);
        Shader* createShaderSync(const ShaderCreateInfo& createInfo);
        Material* createMaterialSync(Shader* shader);
        Texture* createTexture(const math::uvector2& size, TextureFormat format, const uint8* data);

        bool createShader(const ShaderCreateInfo& createInfo, std::function<void(Shader*)> callback);
        bool createMaterial(Shader* shader, std::function<void(Material*)> callback);

        void destroyRenderTarget(RenderTarget* renderTarget);
        void destroyVertexBuffer(VertexBuffer* vertexBuffer);
        void destroyShader(Shader* shader);
        void destroyMaterial(Material* material);
        void destroyTexture(Texture* texture);

        void registerVertexComponent(const jstringID& vertexComponentID, const VertexComponentDescription& description);
        const VertexComponentDescription* findVertexComponent(const jstringID& componentID) const { return m_RegisteredVertexComponents.find(componentID); }
        const RegisteredVertexDescription* findVertex(const vertex_id vertexID) const { return m_RegisteredVerticesData.find(vertexID); }

        Texture* getDefaultTexture() const { return m_DefaultTexture; }

        bool render();

    protected:

        virtual bool initInternal(const WindowCreateInfo& mainWindowInfo);
        virtual bool initAssetLoadingWorker(int32 workerIndex) { return true; }
        virtual void clearAssetLoadingWorker(int32 workerIndex) {}
        virtual void clearInternal() { clearData(); }

        void clearAssets();
        void clearData();

        virtual WindowController* createWindowController() = 0;
        virtual RenderPipeline* createRenderPipelineInternal();
        virtual RenderTarget* allocateRenderTarget() = 0;
        virtual VertexBuffer* allocateVertexBuffer() = 0;
        virtual Shader* allocateShader() = 0;
        virtual Material* allocateMaterial() = 0;
        virtual Texture* allocateTexture() = 0;

        virtual void deallocateRenderTarget(RenderTarget* renderTarget) = 0;
        virtual void deallocateVertexBuffer(VertexBuffer* vertexBuffer) = 0;
        virtual void deallocateShader(Shader* shader) = 0;
        virtual void deallocateMaterial(Material* material) = 0;
        virtual void deallocateTexture(Texture* texture) = 0;

        virtual void onRegisteredVertex(const vertex_id vertexID, const RegisteredVertexDescription& data) {}

    private:

        bool m_Initialized = false;

        WindowController* m_WindowController = nullptr;
        RenderPipeline* m_RenderPipeline = nullptr;

        jmap<render_target_id, RenderTarget*> m_RenderTargets;
        juid<render_target_id> m_RenderTagetIDs;

        jmap<jstringID, VertexComponentDescription> m_RegisteredVertexComponents;
        juid<vertex_id> m_VertexIDGenerator;
        jmap<VertexDescription, vertex_id> m_RegisteredVertices;
        jmap<vertex_id, RegisteredVertexDescription> m_RegisteredVerticesData;

        jtask_queue<> m_AssetLoadingTaskQueue;

        Texture* m_DefaultTexture = nullptr;


        bool createRenderAssets();

        void registerObjectInternal(RenderEngineContextObjectBase* object);

        RenderTarget* createWindowRenderTarget(window_id windowID, TextureSamples samples);
        
        vertex_id registerVertex(const VertexDescription& description);
    };
    
    template<RenderAPI API>
    bool IsSupportRenderAPI() { return false; }
    template<RenderAPI API>
    RenderEngine* CreateRenderEngine() { return nullptr; }
}
