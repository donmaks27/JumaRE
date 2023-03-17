// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../../include/JumaRE/RenderEngineContextObject.h"

#include <mutex>
#include <jutils/jarray.h>
#include <jutils/jlist.h>

namespace JumaRenderEngine
{
    template<typename ObjectType, typename StoreObjectType = ObjectType, TEMPLATE_ENABLE(is_base_and_not_abstract<RenderEngineContextObjectBase, StoreObjectType> && is_base<ObjectType, StoreObjectType>)>
    class RenderEngineObjectsPool
    {
    public:
        RenderEngineObjectsPool() = default;
        ~RenderEngineObjectsPool() { clear(); }

        ObjectType* getObject(RenderEngine* engine)
        {
            m_Mutex.lock();
            if (!m_UnusedObjects.isEmpty())
            {
                ObjectType* object = m_UnusedObjects.getLast();
                m_UnusedObjects.removeLast();
                m_Mutex.unlock();
                return object;
            }
            ObjectType* object = &m_Objects.addDefault();
            m_Mutex.unlock();
            return engine->registerObject(object);
        }
        void returnObject(ObjectType* object)
        {
            if (object != nullptr)
            {
                this->clearObject(object);
                m_Mutex.lock();
                m_UnusedObjects.add(object);
                m_Mutex.unlock();
            }
        }
        void clear()
        {
            m_Mutex.lock();
            m_UnusedObjects.clear();
            jlist<StoreObjectType> objects = std::move(m_Objects);
            m_Mutex.unlock();
            objects.clear();
        }

    private:

        std::mutex m_Mutex;
        jlist<StoreObjectType> m_Objects;
        jarray<ObjectType*> m_UnusedObjects;


        static void clearObject(RenderEngineContextObject* object) { object->clear(); }
        static void clearObject(RenderEngineContextObjectBase* object) {}
    };
}
