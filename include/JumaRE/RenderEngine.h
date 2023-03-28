// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"

#include <jutils/jasync_task_queue.h>

#include "RenderAPI.h"
#include "RenderPrimitivesList.h"
#include "render_target_id.h"
#include "material/ShaderCreateInfo.h"
#include "texture/TextureFormat.h"
#include "vertex/VertexBufferData.h"
#include "window/WindowController.h"

namespace JumaRenderEngine
{
	class Material;
    class RenderEngine;
	class RenderEngineAsset;
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
        int32 assetTaskWorkerCount = 2;
    };

    JUTILS_CREATE_MULTICAST_DELEGATE1(OnRenderEngineEvent, RenderEngine*, renderEngine);

    class RenderEngine
    {
        friend WindowController;

    public:
        RenderEngine() = default;
        virtual ~RenderEngine();

        OnRenderEngineEvent onDestroying;

        template<typename T>
        class OnAssetCreatedTask : public jasync_task
        {
            friend RenderEngine;

        protected:
            OnAssetCreatedTask() = default;
        public:

            T* getAsset() const { return m_Asset; }

        private:

            T* m_Asset = nullptr;
        };


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

        jasync_task_queue_base& getAsyncAssetTaksQueue() { return m_AsyncAssetTaskQueue; }
        RenderPipeline* getRenderPipeline() const { return m_RenderPipeline; }

        // Create functions should be called only from main thread

        bool createShaderAsync(const ShaderCreateInfo& createInfo, const std::function<void(Shader*)>& callback);
        bool createShaderAsync(const ShaderCreateInfo& createInfo, OnAssetCreatedTask<Shader>* onAssetCreated);
        Shader* createShader(const ShaderCreateInfo& createInfo);
        void destroyShader(Shader* shader);

        Material* createMaterial(Shader* shader);
        void destroyMaterial(Material* material);

        void registerVertexComponent(const jstringID& vertexComponentID, const VertexComponentDescription& description);
        const VertexComponentDescription* findVertexComponent(const jstringID& componentID) const { return m_RegisteredVertexComponents.find(componentID); }
        const RegisteredVertexDescription* findVertex(const vertex_id vertexID) const { return m_RegisteredVerticesData.find(vertexID); }
        VertexBuffer* createVertexBuffer(const VertexBufferData& data);
        void destroyVertexBuffer(VertexBuffer* vertexBuffer);

        Texture* createTexture(const math::uvector2& size, TextureFormat format, const uint8* data);
        void destroyTexture(Texture* texture);
        Texture* getDefaultTexture() const { return m_DefaultTexture; }

        RenderTarget* getRenderTarget(render_target_id renderTargetID) const;
        RenderTarget* createRenderTarget(TextureFormat format, const math::uvector2& size, TextureSamples samples);
        void destroyRenderTarget(RenderTarget* renderTarget);

        void destroyAsset(RenderEngineAsset* asset);

        bool render();

    protected:

        virtual bool initInternal(const WindowCreateInfo& mainWindowInfo);
        virtual bool initAsyncAssetTaskQueueWorker(int32 workerIndex) { return true; }
        virtual bool initAsyncAssetTaskQueueWorkerThread(int32 workerIndex) { return true; }
        virtual void clearAsyncAssetTaskQueueWorkerThread(int32 workerIndex) {}
        virtual void clearAsyncAssetTaskQueueWorker(int32 workerIndex) {}
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

        class AsyncAssetWorker : public jasync_worker
        {
        public:
            AsyncAssetWorker() = delete;
            AsyncAssetWorker(RenderEngine* renderEngine) : m_RenderEngine(renderEngine) {}

            bool onStart_MainThread() const { return m_RenderEngine->initAsyncAssetTaskQueueWorker(getWorkerIndex()); }
            bool onStart_WorkerThread() const { return m_RenderEngine->initAsyncAssetTaskQueueWorkerThread(getWorkerIndex()); }
            void onStop_WorkerThread() const { m_RenderEngine->clearAsyncAssetTaskQueueWorkerThread(getWorkerIndex()); }
            void onStop_MainThread() const { m_RenderEngine->clearAsyncAssetTaskQueueWorker(getWorkerIndex()); }

        private:

            RenderEngine* m_RenderEngine = nullptr;
        };
        class AsyncAssetCreateTask : public jasync_task_default
        {
        public:
            AsyncAssetCreateTask() = delete;
            AsyncAssetCreateTask(const std::function<void()>& func, jasync_task* onFinish)
                : jasync_task_default(func), m_OnFinishTask(onFinish)
            {}
            AsyncAssetCreateTask(std::function<void()>&& func, jasync_task* onFinish) noexcept
                : jasync_task_default(std::move(func)), m_OnFinishTask(onFinish)
            {}
            virtual ~AsyncAssetCreateTask() override
            {
                if ((m_OnFinishTask != nullptr) && m_OnFinishTask->shouldDeleteAfterExecution())
                {
                    delete m_OnFinishTask;
                }
            }

            virtual void run() override
            {
                jasync_task_default::run();
                if (m_OnFinishTask != nullptr)
                {
                    m_OnFinishTask->run();
                }
            }

        private:

            jasync_task* m_OnFinishTask = nullptr;
        };
        class AsyncAssetDestroyTask : public jasync_task
        {
            friend RenderEngine;

        public:
            AsyncAssetDestroyTask() = delete;
            AsyncAssetDestroyTask(RenderEngineAsset* asset) : m_Asset(asset) {}

            virtual void run() override;
            virtual bool shouldDeleteAfterExecution() const override { return false; }

        private:

            RenderEngineAsset* m_Asset = nullptr;
            std::atomic_bool m_TaskFinished = false;
        };

        jasync_task_queue<AsyncAssetWorker> m_AsyncAssetTaskQueue;
        std::mutex m_RenderAssets_MarkedForDestroyMutex;
        jlist<std::pair<RenderEngineAsset*, uint8>> m_RenderAssets_MarkedForDestroy;
        jlist<AsyncAssetDestroyTask> m_RenderAssets_DestroyTasks;
        jarray<jasync_task*> m_RenderAssets_DestroyTasksTemp;

        jmap<render_target_id, RenderTarget*> m_RenderTargets;
        jmap<jstringID, VertexComponentDescription> m_RegisteredVertexComponents;
        jmap<VertexDescription, vertex_id> m_RegisteredVertices;
        jmap<vertex_id, RegisteredVertexDescription> m_RegisteredVerticesData;
        
        WindowController* m_WindowController = nullptr;
        RenderPipeline* m_RenderPipeline = nullptr;
        Texture* m_DefaultTexture = nullptr;
        
        juid<render_target_id> m_RenderTagetIDs;
        juid<vertex_id> m_VertexIDGenerator;
        
        bool m_Initialized = false;


        bool createRenderAssets();

        void registerObjectInternal(RenderEngineContextObjectBase* object);

        RenderTarget* createWindowRenderTarget(window_id windowID, TextureSamples samples);
        
        vertex_id registerVertex(const VertexDescription& description);

        void processMarkedForDestroyAssets();
        void processFinishedDestroyAssetTasks();
    };
    
    template<RenderAPI API>
    bool IsSupportRenderAPI() { return false; }
    template<RenderAPI API>
    RenderEngine* CreateRenderEngine() { return nullptr; }
}
