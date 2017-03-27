/*
 * VKPtr.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PTR_H
#define LLGL_VK_PTR_H


#include <functional>
#include <vulkan/vulkan.h>


namespace LLGL
{


// Wrapper class for Vulkan objects (similar to ComPtr in DirectX).
template <typename T>
class VKPtr
{

    public:

        VKPtr() throw() :
            VKPtr([](T, VkAllocationCallbacks*) {})
        {
        }

        VKPtr(const std::function<void(T, VkAllocationCallbacks*)>& deleter) throw()
        {
            deleter_ = [=](T obj)
            {
                deleter(obj, nullptr);
            };
        }

        VKPtr(const VKPtr<VkInstance>& instance, const std::function<void(VkInstance, T, VkAllocationCallbacks*)>& deleter) throw()
        {
            deleter_ = [&instance, deleter](T obj)
            {
                deleter(instance, obj, nullptr);
            };
        }

        VKPtr(const VKPtr<VkDevice>& device, const std::function<void(VkDevice, T, VkAllocationCallbacks*)>& deleter) throw() :
        {
            deleter_ = [&device, deleter](T obj)
            {
                deleter(device, obj, nullptr);
            };
        }

        ~VKPtr()
        {
            Release();
        }

        const T* operator & () const throw()
        {
            return &object_;
        }

        T* ReleaseAndGetAddressOf()
        {
            Release();
            return &object_;
        }

        operator T () const throw()
        {
            return object_;
        }

        VKPtr<T>& operator = (const T& rhs)
        {
            Release();
            object_ = rhs;
            return *this;
        }

        template <typename U>
        bool operator == (const U& rhs) const throw()
        {
            return (object_ == rhs);
        }

    private:

        T                       object_ { VK_NULL_HANDLE };
        std::function<void(T)>  deleter_;

        void Release()
        {
            if (object_ != VK_NULL_HANDLE)
            {
                deleter_(object_);
                object_ = VK_NULL_HANDLE;
            }
        }

};


} // /namespace LLGL


#endif



// ================================================================================
