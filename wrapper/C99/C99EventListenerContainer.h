/*
 * C99EventListenerContainer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_EVENT_LISTENER_CONTAINER_H
#define LLGL_C99_EVENT_LISTENER_CONTAINER_H


#include "../../sources/Core/Assertion.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include <limits.h>


template <typename TEventListener, typename TWrapperCallbacks>
class EventListenerContainer
{

    public:

        using TEventListenerSPtr = std::shared_ptr<TEventListener>;

        std::pair<int, TEventListenerSPtr> Create(const TWrapperCallbacks* callbacks)
        {
            std::lock_guard<std::mutex> guard{ mutex_ };
            LLGL_ASSERT(idCounter_ < INT_MAX);
            const int id = ++idCounter_;
            std::pair<int, TEventListenerSPtr> result{ id, std::make_shared<TEventListener>(callbacks) };
            eventListeners_.insert(result);
            return result;
        }

        TEventListenerSPtr Release(int id)
        {
            std::lock_guard<std::mutex> guard{ mutex_ };
            auto it = eventListeners_.find(id);
            if (it != eventListeners_.end())
            {
                TEventListenerSPtr result = std::move(it->second);
                eventListeners_.erase(it);
                return result;
            }
            return nullptr;
        }

    private:

        int                                         idCounter_      = 0;
        std::unordered_map<int, TEventListenerSPtr> eventListeners_;
        std::mutex                                  mutex_;

};


#endif



// ================================================================================
