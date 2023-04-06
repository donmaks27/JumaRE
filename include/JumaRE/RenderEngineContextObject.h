// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"

#include <jutils/jpool_simple.h>

namespace JumaRenderEngine
{
    class RenderEngine;

    class RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        RenderEngineContextObjectBase() = default;
        virtual ~RenderEngineContextObjectBase() = default;

        RenderEngine* getRenderEngine() const { return s_RenderEngine; }
        template<typename T, TEMPLATE_ENABLE(is_base<RenderEngine, T>)>
        T* getRenderEngine() const { return dynamic_cast<T*>(getRenderEngine()); }

    private:

        static RenderEngine* s_RenderEngine;
    };

    class RenderEngineContextObject : public RenderEngineContextObjectBase, public jpool_simple_object
    {
        friend RenderEngine;

    public:
        RenderEngineContextObject() = default;
        virtual ~RenderEngineContextObject() override = default;

        bool isValid() const { return m_Initialized; }
        void clear()
        {
            if (isValid())
            {
                clearInternal();
                m_Initialized = false;
            }
        }

    protected:

        void markAsInitialized() { m_Initialized = true; }

        virtual void clearInternal() {}

        virtual void clearPoolObject() override { clear(); }

    private:

        bool m_Initialized = false;
    };
}
