// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../include/JumaRE/RenderEngine.h"

#include <vma/vk_mem_alloc.h>

#include "Material_Vulkan.h"
#include "RenderTarget_Vulkan.h"
#include "Shader_Vulkan.h"
#include "Texture_Vulkan.h"
#include "VertexBuffer_Vulkan.h"
#include "vulkanObjects/VulkanBuffer.h"
#include "vulkanObjects/VulkanImage.h"
#include "vulkanObjects/VulkanRenderPass.h"
#include "../utils/RenderEngineObjectsPool.h"

namespace JumaRenderEngine
{
    class VulkanRenderPass;
    class VulkanImage;
    class VulkanBuffer;
    class VulkanCommandPool;

    struct VulkanQueueDescription
    {
        uint32 familyIndex = 0;
        uint32 queueIndex = 0;
        VkQueue queue = nullptr;
    };
    
    struct VertexDescription_Vulkan
    {
        VkVertexInputBindingDescription binding = VkVertexInputBindingDescription();
        jarray<VkVertexInputAttributeDescription> attributes;
    };

    class RenderEngine_Vulkan final : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_Vulkan() = default;
        virtual ~RenderEngine_Vulkan() override;

        virtual RenderAPI getRenderAPI() const override { return RenderAPI::Vulkan; }

        VkInstance getVulkanInstance() const { return m_VulkanInstance; }
        VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
        VkDevice getDevice() const { return m_Device; }
        VmaAllocator getAllocator() const { return m_Allocator; }

        const VulkanQueueDescription* getQueue(const VulkanQueueType type) const { return !m_QueueIndices.isEmpty() ? &m_Queues[m_QueueIndices[type]] : nullptr; }
        VulkanCommandPool* getCommandPool(const VulkanQueueType type) const { return !m_CommandPools.isEmpty() ? m_CommandPools[type] : nullptr; }

        VulkanBuffer* getVulkanBuffer() { return m_VulkanBuffersPool.getObject(this); }
        VulkanImage* getVulkanImage() { return m_VulkanImagesPool.getObject(this); }
        void returnVulkanBuffer(VulkanBuffer* buffer) { m_VulkanBuffersPool.returnObject(buffer); }
        void returnVulkanImage(VulkanImage* image) { m_VulkanImagesPool.returnObject(image); }

        VulkanRenderPass* getRenderPass(const VulkanRenderPassDescription& description);
        const VulkanRenderPassDescription* findRenderPassDescription(render_pass_type_id renderPassID) const;

        const VertexDescription_Vulkan* findVertexType_Vulkan(const vertex_id vertexID) const { return m_RegisteredVertices_Vulkan.find(vertexID); }

        VkSampler getTextureSampler(TextureSamplerType samplerType);

    protected:

        virtual bool initInternal(const WindowCreateInfo& mainWindowInfo) override;
        virtual void clearInternal() override;

        virtual WindowController* createWindowController() override;
        virtual RenderPipeline* createRenderPipelineInternal() override;
        virtual RenderTarget* allocateRenderTarget() override { return m_RenderTargetsPool.getObject(this); }
        virtual VertexBuffer* allocateVertexBuffer() override { return m_VertexBuffersPool.getObject(this); }
        virtual Shader* allocateShader() override { return m_ShadersPool.getObject(this); }
        virtual Material* allocateMaterial() override { return m_MaterialsPool.getObject(this); }
        virtual Texture* allocateTexture() override { return m_TexturesPool.getObject(this); }

        virtual void deallocateRenderTarget(RenderTarget* renderTarget) override { m_RenderTargetsPool.returnObject(renderTarget); }
        virtual void deallocateVertexBuffer(VertexBuffer* vertexBuffer) override { m_VertexBuffersPool.returnObject(vertexBuffer); }
        virtual void deallocateShader(Shader* shader) override { m_ShadersPool.returnObject(shader); }
        virtual void deallocateMaterial(Material* material) override { m_MaterialsPool.returnObject(material); }
        virtual void deallocateTexture(Texture* texture) override { m_TexturesPool.returnObject(texture); }

        virtual void onRegisteredVertex(vertex_id vertexID, const RegisteredVertexDescription& data) override;

    private:

        static constexpr uint8 m_RequiredExtensionCount = 1;
        static constexpr const char* m_RequiredExtensions[m_RequiredExtensionCount] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkInstance m_VulkanInstance = nullptr;
#ifdef JDEBUG
        VkDebugUtilsMessengerEXT m_DebugMessenger = nullptr;
        static VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
            VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif

        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkDevice m_Device = nullptr;
        VmaAllocator m_Allocator = nullptr;

        jmap<VulkanQueueType, int32> m_QueueIndices;
        jarray<VulkanQueueDescription> m_Queues;
        jmap<VulkanQueueType, VulkanCommandPool*> m_CommandPools;
        
        RenderEngineObjectsPool<RenderTarget, RenderTarget_Vulkan> m_RenderTargetsPool;
        RenderEngineObjectsPool<VertexBuffer, VertexBuffer_Vulkan> m_VertexBuffersPool;
        RenderEngineObjectsPool<Shader, Shader_Vulkan> m_ShadersPool;
        RenderEngineObjectsPool<Material, Material_Vulkan> m_MaterialsPool;
        RenderEngineObjectsPool<Texture, Texture_Vulkan> m_TexturesPool;

        RenderEngineObjectsPool<VulkanBuffer> m_VulkanBuffersPool;
        RenderEngineObjectsPool<VulkanImage> m_VulkanImagesPool;
        
        juid<render_pass_type_id> m_RenderPassTypeIDs;
        jmap<VulkanRenderPassDescription, render_pass_type_id, VulkanRenderPassDescription::compatible_predicate> m_RenderPassTypes;
        jmap<VulkanRenderPassDescription, VulkanRenderPass, VulkanRenderPassDescription::equal_predicate> m_RenderPasses;

        jmap<vertex_id, VertexDescription_Vulkan> m_RegisteredVertices_Vulkan;

        jmap<TextureSamplerType, VkSampler> m_TextureSamplers;


        bool createVulkanInstance();
        jarray<const char*> getRequiredVulkanExtensions() const;

        bool pickPhysicalDevice();
        static bool getQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, 
            jmap<VulkanQueueType, int32>& outQueueIndices, jarray<VulkanQueueDescription>& outQueues);
        bool createDevice();
        bool createCommandPools();

        void clearVulkan();
    };
}

#endif
