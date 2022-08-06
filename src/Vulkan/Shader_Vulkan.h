// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../include/JumaRE/material/Shader.h"

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    class Shader_Vulkan final : public Shader
    {
        using Super = Shader;

    public:
        Shader_Vulkan() = default;
        virtual ~Shader_Vulkan() override;
        
        VkDescriptorSetLayout getDescriptorSetLayout() const { return m_DescriptorSetLayout; }
        VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }

        const jarray<VkPipelineShaderStageCreateInfo>& getPipelineStageInfos() const { return m_CachedPipelineStageInfos; }

    protected:

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) override;

    private:

        jmap<ShaderStageFlags, VkShaderModule> m_ShaderModules;
        VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
        VkPipelineLayout m_PipelineLayout = nullptr;

        jarray<VkPipelineShaderStageCreateInfo> m_CachedPipelineStageInfos;


        bool createShaderModules(VkDevice device, const jmap<ShaderStageFlags, jstring>& fileNames);
        bool createDescriptorSetLayout(VkDevice device);
        bool createPipelineLayout(VkDevice device);

        void clearVulkan();
    };
}

#endif
