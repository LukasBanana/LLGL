/*
 * VKPtr.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PTR_H
#define LLGL_VK_PTR_H


#include <functional>
#include "Vulkan.h"


namespace LLGL
{


// Wrapper class for Vulkan objects (similar to ComPtr in DirectX).
template <typename T>
class VKPtr
{

    public:

        VKPtr() :
            VKPtr { [](T, VkAllocationCallbacks*) {} }
        {
        }

        VKPtr(const std::function<void(T, VkAllocationCallbacks*)>& deleter)
        {
            deleter_ = [=](T obj)
            {
                deleter(obj, nullptr);
            };
        }

        VKPtr(const VKPtr<VkInstance>& instance, const std::function<void(VkInstance, T, VkAllocationCallbacks*)>& deleter)
        {
            deleter_ = [&instance, deleter](T obj)
            {
                deleter(instance, obj, nullptr);
            };
        }

        VKPtr(const VKPtr<VkDevice>& device, const std::function<void(VkDevice, T, VkAllocationCallbacks*)>& deleter)
        {
            deleter_ = [&device, deleter](T obj)
            {
                deleter(device, obj, nullptr);
            };
        }

        VKPtr(const VKPtr<T>&) = default;

        VKPtr(VKPtr<T>&& rhs) :
            object_  { rhs.object_  },
            deleter_ { rhs.deleter_ }
        {
            rhs.object_ = VK_NULL_HANDLE;
        }

        ~VKPtr()
        {
            Release();
        }

        const T* operator & () const
        {
            return &object_;
        }

        T* operator & ()
        {
            return &object_;
        }

        void Release()
        {
            if (object_ != VK_NULL_HANDLE)
            {
                deleter_(object_);
                object_ = VK_NULL_HANDLE;
            }
        }

        T* ReleaseAndGetAddressOf()
        {
            Release();
            return &object_;
        }

        /*VKPtr<T>& TakeOwnership(VKPtr<T>& rhs)
        {
            object_ = rhs.object_;
            deleter_ = rhs.deleter_;
            rhs.object_ = VK_NULL_HANDLE;
            return *this;
        }*/

        inline T Get() const
        {
            return object_;
        }

        operator T () const
        {
            return object_;
        }

        VKPtr<T>& operator = (const T& rhs)
        {
            Release();
            object_ = rhs;
            return *this;
        }

        VKPtr<T>& operator = (VKPtr&& rhs)
        {
            object_ = rhs.object_;
            rhs.object_ = VK_NULL_HANDLE;
            return *this;
        }

        template <typename U>
        bool operator == (const U& rhs) const
        {
            return (object_ == rhs);
        }

    private:

        T                       object_ { VK_NULL_HANDLE };
        std::function<void(T)>  deleter_;

};


} // /namespace LLGL


#endif



// ================================================================================
