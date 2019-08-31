/*
 * VKPtr.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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

        VKPtr(const VKPtr<T>&) = delete;
        VKPtr<T>& operator = (const VKPtr& rhs) = delete;

        // Default constructor with dummy deleter.
        VKPtr() :
            VKPtr { [](T, VkAllocationCallbacks*) {} }
        {
        }

        // Constructs the handler with the specified deleter function.
        VKPtr(const std::function<void(T, VkAllocationCallbacks*)>& deleter)
        {
            deleter_ = [=](T obj)
            {
                deleter(obj, nullptr);
            };
        }

        // Constructs the handler with the specified deleter function and the Vulkan instance.
        VKPtr(
            const VKPtr<VkInstance>&                                            instance,
            const std::function<void(VkInstance, T, VkAllocationCallbacks*)>&   deleter)
        {
            deleter_ = [&instance, deleter](T obj)
            {
                deleter(instance, obj, nullptr);
            };
        }

        // Constructs the handler with the specified deleter function and the Vulkan device.
        VKPtr(
            const VKPtr<VkDevice>&                                          device,
            const std::function<void(VkDevice, T, VkAllocationCallbacks*)>& deleter)
        {
            deleter_ = [&device, deleter](T obj)
            {
                deleter(device, obj, nullptr);
            };
        }

        // Moves the native Vulkan object of the specified handler into this handler.
        VKPtr(VKPtr<T>&& rhs) :
            object_  { rhs.object_  },
            deleter_ { rhs.deleter_ }
        {
            rhs.object_ = VK_NULL_HANDLE;
        }

        // Releases the native Vulkan object.
        ~VKPtr()
        {
            Release();
        }

        // Returns a constant pointer to the native Vulkan object.
        const T* operator & () const
        {
            return &object_;
        }

        // Returns a pointer to the native Vulkan object.
        T* operator & ()
        {
            return &object_;
        }

        // Deletes the native Vulkan object using the respective deleter function.
        void Release()
        {
            if (object_ != VK_NULL_HANDLE)
            {
                deleter_(object_);
                object_ = VK_NULL_HANDLE;
            }
        }

        // Releases and returns the address of the native Vulkan object.
        T* ReleaseAndGetAddressOf()
        {
            Release();
            return &object_;
        }

        // Returns the native Vulkan object.
        inline T Get() const
        {
            return object_;
        }

        // Returns the native Vulkan object (shortcut for "Get()").
        operator T () const
        {
            return object_;
        }

        // Release the object and takes the specified native Vulkan object.
        VKPtr<T>& operator = (const T& rhs)
        {
            Release();
            object_ = rhs;
            return *this;
        }

        // Moves the specified VKPtr handler into this handler.
        VKPtr<T>& operator = (VKPtr&& rhs)
        {
            object_ = rhs.object_;
            rhs.object_ = VK_NULL_HANDLE;
            return *this;
        }

        // Returns true if this handler contains the same native Vulkan object as the specified parameter.
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
