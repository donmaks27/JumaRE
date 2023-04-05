// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "JumaRE/RenderEngineContextObject.h"

#include <jutils/jarray.h>
#include <jutils/jlist.h>

#include <mutex>

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
        
        jlist<StoreObjectType> m_Objects;
        jarray<ObjectType*> m_UnusedObjects;


        static void clearObject(RenderEngineContextObject* object) { object->clear(); }
        static void clearObject(RenderEngineContextObjectBase*) {}
    };

    template<typename ObjectType, typename StoreObjectType = ObjectType, TEMPLATE_ENABLE(is_base_and_not_abstract<RenderEngineContextObjectBase, StoreObjectType> && is_base<ObjectType, StoreObjectType>)>
    class RenderEngineObjectsPoolAsync : public RenderEngineObjectsPool<ObjectType, StoreObjectType>
    {
        using Super = RenderEngineObjectsPool<ObjectType, StoreObjectType>;

    public:
        RenderEngineObjectsPoolAsync() = default;
        ~RenderEngineObjectsPoolAsync() { clear(); }

        ObjectType* getObject(RenderEngine* engine)
        {
            std::lock_guard lock(m_Mutex);
            return Super::getObject(engine);
        }
        void returnObject(ObjectType* object)
        {
            std::lock_guard lock(m_Mutex);
            return Super::returnObject(object);
        }
        void clear()
        {
            std::lock_guard lock(m_Mutex);
            Super::clear();
        }

    private:

        std::mutex m_Mutex;
    };
}
