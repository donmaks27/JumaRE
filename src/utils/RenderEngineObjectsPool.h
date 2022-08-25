// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../../include/JumaRE/RenderEngineContextObject.h"

#include <jutils/jarray.h>
#include <jutils/jlist.h>

namespace JumaRenderEngine
{
    template<typename ObjectType, TEMPLATE_ENABLE(is_base_and_not_abstract<RenderEngineContextObjectBase, ObjectType>)>
    class RenderEngineObjectsPool
    {
    public:
        RenderEngineObjectsPool() = default;
        ~RenderEngineObjectsPool() = default;

        ObjectType* getObject(RenderEngine* engine)
        {
            if (!m_UnusedObjects.isEmpty())
            {
                ObjectType* object = m_UnusedObjects.getLast();
                m_UnusedObjects.removeLast();
                return object;
            }
            return engine->registerObject(&m_Objects.addDefault());
        }
        void returnObject(ObjectType* object)
        {
            if (object != nullptr)
            {
                this->clearObject(object);
                m_UnusedObjects.add(object);
            }
        }
        void clear()
        {
            m_UnusedObjects.clear();
            m_Objects.clear();
        }

    private:

        jlist<ObjectType> m_Objects;
        jarray<ObjectType*> m_UnusedObjects;


        static void clearObject(RenderEngineContextObject* object) { object->clear(); }
        static void clearObject(RenderEngineContextObjectBase* object) {}
    };
}
