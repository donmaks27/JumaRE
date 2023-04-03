// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "JumaRE/material/Material.h"

#include <jutils/jasync_task_queue.h>

namespace JumaRenderEngine
{
	struct RenderOptions;

	class Material_OpenGL final : public Material
    {
        using Super = Material;

    public:
        Material_OpenGL() = default;
        virtual ~Material_OpenGL() override;

        bool bindMaterial(const RenderOptions* renderOptions);
        void unbindMaterial();

    protected:

        virtual bool initInternal() override;

        virtual bool isReadyForDestroy() override { return !m_CreateTaskActive; }
        virtual void onClearAsset() override;

    private:

        class CreateMaterialTask : public jasync_task
        {
        public:
            CreateMaterialTask() = delete;
            CreateMaterialTask(Material_OpenGL* material) : m_Material(material) {}
            virtual ~CreateMaterialTask() override { m_Material->m_CreateTaskActive = false; }

            virtual void run() override { m_Material->createUniformBuffers(); }

        private:
            
            Material_OpenGL* m_Material = nullptr;
        };

        jmap<uint32, uint32> m_UniformBufferIndices;
        
        std::atomic_bool m_CreateTaskActive = false;
        bool m_MaterialCreated = false;


        void createUniformBuffers();

	    void clearOpenGL();

        void updateUniformData();
    };
}

#endif
