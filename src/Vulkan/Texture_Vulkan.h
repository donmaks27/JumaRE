// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../include/JumaRE/texture/Texture.h"

namespace JumaRenderEngine
{
    class VulkanImage;

    class Texture_Vulkan final : public Texture
    {
    public:
        Texture_Vulkan() = default;
        virtual ~Texture_Vulkan() override;

        VulkanImage* getVulkanImage() const { return m_Image; }

    protected:

        virtual bool initInternal(const math::uvector2& size, TextureFormat format, const uint8* data) override;

    private:

        VulkanImage* m_Image = nullptr;


        void clearVulkan();
    };
}

#endif
