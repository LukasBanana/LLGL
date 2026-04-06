/*
 * WGPtr.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_PTR_H
#define LLGL_WG_PTR_H


#include <functional>


namespace LLGL
{


// Wrapper class for Vulkan objects (similar to ComPtr in DirectX).
template <typename T>
class WGPtr
{

    public:

        WGPtr(const WGPtr<T>&) = delete;
        WGPtr<T>& operator = (const WGPtr& rhs) = delete;

        // Default constructor with dummy deleter.
        WGPtr() :
            WGPtr { [](T) {} }
        {
        }

        // Constructs a default WGPtr object when initialized with nullptr.
        WGPtr(std::nullptr_t) :
            WGPtr { [](T) {} }
        {
        }

        // Constructs the handler with the specified deleter function.
        WGPtr(const std::function<void(T)>& deleter) :
            deleter_ { deleter }
        {
        }

        // Moves the native Vulkan object of the specified handler into this handler.
        WGPtr(WGPtr<T>&& rhs) :
            object_  { rhs.object_  },
            deleter_ { rhs.deleter_ }
        {
            rhs.object_  = nullptr;
            rhs.deleter_ = nullptr;
        }

        // Releases the native Vulkan object.
        ~WGPtr()
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
            if (object_ != nullptr)
            {
                deleter_(object_);
                object_ = nullptr;
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
        WGPtr<T>& operator = (const T& rhs)
        {
            Release();
            object_ = rhs;
            return *this;
        }

        // Moves the specified WGPtr handler into this handler.
        WGPtr<T>& operator = (WGPtr&& rhs)
        {
            if (this != &rhs)
            {
                Release();
                object_ = rhs.object_;
                deleter_ = rhs.deleter_;
                rhs.object_ = nullptr;
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

        T                       object_ { nullptr };
        std::function<void(T)>  deleter_;

};


} // /namespace LLGL


#endif



// ================================================================================
