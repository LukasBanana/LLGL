/*
 * VKPtr.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        // Constructs a default VKPtr object when initialized with a VK_NULL_HANDLE.
        VKPtr(std::nullptr_t) :
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
            VkInstance                                                          instance,
            const std::function<void(VkInstance, T, VkAllocationCallbacks*)>&   deleter)
        {
            deleter_ = [instance, deleter](T obj)
            {
                deleter(instance, obj, nullptr);
            };
        }

        // Constructs the handler with the specified deleter function and the Vulkan device.
        VKPtr(
            VkDevice                                                        device,
            const std::function<void(VkDevice, T, VkAllocationCallbacks*)>& deleter)
        {
            deleter_ = [device, deleter](T obj)
            {
                deleter(device, obj, nullptr);
            };
        }

        // Constructs the hanlder with bypassing the deleter. Only used for custom native handle support, in which case the object is interpreted as a weak reference.
        explicit VKPtr(T obj) :
            object_  { obj                },
            deleter_ { [](T){ /*dummy*/ } }
        {
        }

        // Moves the native Vulkan object of the specified handler into this handler.
        VKPtr(VKPtr<T>&& rhs) :
            object_  { rhs.object_  },
            deleter_ { rhs.deleter_ }
        {
            rhs.object_ = VK_NULL_HANDLE;
            rhs.deleter_ = nullptr;
        }

        // Releases the native Vulkan object.
        ~VKPtr()
        {
            Release();
        }

        // Returns a constant pointer to the native Vulkan object.
        const T* GetAddressOf() const
        {
            return &object_;
        }

        // Returns a pointer to the native Vulkan object.
        T* GetAddressOf()
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
            if (this != &rhs)
            {
                Release();
                object_ = rhs.object_;
                deleter_ = rhs.deleter_;
                rhs.object_ = VK_NULL_HANDLE;
                rhs.deleter_ = nullptr;
            }
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
