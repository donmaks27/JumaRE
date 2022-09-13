// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"

#include <jutils/jset.h>

#include "RenderAPI.h"
#include "material/ShaderUniform.h"
#include "texture/TextureFormat.h"
#include "texture/TextureSamples.h"
#include "vertex/VertexDescription.h"
#include "window/WindowController.h"

namespace JumaRenderEngine
{
    class Material;
    class RenderPipeline;
    class Shader;
    class Texture;
    class VertexBuffer;
    class VertexBufferData;

    class RenderEngine
    {
        friend WindowController;

    public:
        RenderEngine() = default;
        virtual ~RenderEngine();

        virtual RenderAPI getRenderAPI() const = 0;

        bool init(const WindowCreateInfo& mainWindowInfo);
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
        template<typename T, TEMPLATE_ENABLE(is_base<RenderPipeline, T>)>
        T* getRenderPipeline() const { return dynamic_cast<T*>(getRenderPipeline()); }

        RenderTarget* createRenderTarget(TextureFormat format, const math::uvector2& size, TextureSamples samples);
        VertexBuffer* createVertexBuffer(VertexBufferData* verticesData);
        Shader* createShader(const jmap<ShaderStageFlags, jstring>& fileNames, jset<jstringID> vertexComponents, jmap<jstringID, ShaderUniform> uniforms = {});
        Material* createMaterial(Shader* shader);
        Texture* createTexture(const math::uvector2& size, TextureFormat format, const uint8* data);

        void destroyRenderTarget(RenderTarget* renderTarget);
        void destroyVertexBuffer(VertexBuffer* vertexBuffer);
        void destroyShader(Shader* shader);
        void destroyMaterial(Material* material);
        void destroyTexture(Texture* texture);

        const VertexDescription* findVertexType(const jstringID& vertexName) const { return m_RegisteredVertexTypes.find(vertexName); }
        virtual math::vector2 getScreenCoordinateModifier() const { return { 1.0f, 1.0f }; }
        virtual bool shouldFlipLoadedTextures() const { return false; }

        Texture* getDefaultTexture() const { return m_DefaultTexture; }

    protected:

        virtual bool initInternal(const WindowCreateInfo& mainWindowInfo);
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

        virtual void onRegisteredVertexType(const jstringID& vertexName) {}

    private:

        bool m_Initialized = false;

        WindowController* m_WindowController = nullptr;
        RenderPipeline* m_RenderPipeline = nullptr;
        jmap<jstringID, VertexDescription> m_RegisteredVertexTypes;

        Texture* m_DefaultTexture = nullptr;


        bool createRenderAssets();

        void registerObjectInternal(RenderEngineContextObjectBase* object);
        
        const VertexDescription* registerVertexType(const VertexBufferData* verticesData);

        RenderTarget* createWindowRenderTarget(window_id windowID, TextureSamples samples);
    };

    template<RenderAPI API>
    RenderEngine* CreateRenderEngine() { return nullptr; }
    inline RenderEngine* CreateRenderEngine(const RenderAPI api)
    {
        switch (api)
        {
        case RenderAPI::Vulkan: return CreateRenderEngine<RenderAPI::Vulkan>();
        case RenderAPI::OpenGL: return CreateRenderEngine<RenderAPI::OpenGL>();
        case RenderAPI::DirectX11: return CreateRenderEngine<RenderAPI::DirectX11>();
        case RenderAPI::DirectX12: return CreateRenderEngine<RenderAPI::DirectX12>();
        default: ;
        }
        return nullptr;
    }
}
