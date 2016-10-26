/*
 * ComPtr.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COM_PTR_H
#define LLGL_COM_PTR_H


#include <Windows.h>
#include <algorithm>


namespace LLGL
{


//! Smart pointer class for COM (Component Object Model) objects.
template <typename T>
class ComPtr
{

    public:

        ComPtr() throw() :
            ptr_( nullptr )
        {
        }

        ComPtr(decltype(nullptr)) throw() :
            ptr_( nullptr )
        {
        }

        template <typename U>
        ComPtr(U* rhs) throw() :
            ptr_( rhs )
        {
            AddRef();
        }

        ComPtr(const ComPtr& rhs) throw() :
            ptr_( rhs.ptr_ )
        {
            AddRef();
        }

        ComPtr(ComPtr&& rhs) throw() :
            ptr_( nullptr )
        {
            if (this != reinterpret_cast<ComPtr*>(&reinterpret_cast<unsigned char&>(rhs)))
                Swap(rhs);
        }

        ~ComPtr() throw()
        {
            Release();
        }

        T* Get() const throw()
        {
            return ptr_;
        }

        T* operator -> () const throw()
        {
            return ptr_;
        }

        operator bool () const throw()
        {
            return (ptr_ != nullptr);
        }

        ComPtr& operator = (decltype(nullptr)) throw()
        {
            Release();
            return *this;
        }

        ComPtr& operator = (T* rhs) throw()
        {
            if (ptr_ != rhs)
                ComPtr(rhs).Swap(*this);
            return *this;
        }

        template <typename U>
        ComPtr& operator = (U* rhs) throw()
        {
            ComPtr(rhs).Swap(*this);
            return *this;
        }

        ComPtr& operator = (const ComPtr& rhs) throw()
        {
            if (ptr_ != rhs.ptr_)
                ComPtr(rhs).Swap(*this);
            return *this;
        }

        template <typename U>
        ComPtr& operator = (const ComPtr<U>& rhs) throw()
        {
            ComPtr(rhs).Swap(*this);
            return *this;
        }

        ComPtr& operator = (ComPtr&& rhs) throw()
        {
            ComPtr(static_cast<ComPtr&&>(rhs)).Swap(*this);
            return *this;
        }

        template <typename U>
        ComPtr& operator = (ComPtr<U>&& rhs) throw()
        {
            ComPtr(static_cast<ComPtr<U>&&>(rhs)).Swap(*this);
            return *this;
        }

        T* const* operator & () const throw()
        {
            return GetAddressOf();
        }

        T** operator & () throw()
        {
            return GetAddressOf();
        }

        //! Returns the constant reference of the internal pointer.
        T* const* GetAddressOf() const throw()
        {
            return &ptr_;
        }

        //! Returns the reference of the internal pointer.
        T** GetAddressOf() throw()
        {
            return &ptr_;
        }

        //! Release the reference and returns the reference of the internal pointer.
        T** ReleaseAndGetAddressOf() throw()
        {
            Release();
            return &ptr_;
        }

        //! Detaches the internal pointer from this smart COM pointer and returns this internal pointer.
        T* Detach() throw()
        {
            auto ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        unsigned long Reset()
        {
            return Release();
        }

        void Swap(ComPtr&& rhs) throw()
        {
            std::swap(ptr_, rhs.ptr_);
        }

        void Swap(ComPtr& rhs) throw()
        {
            std::swap(ptr_, rhs.ptr_);
        }

        template <typename U>
        HRESULT As(ComPtr<U>& ptr) const throw()
        {
            return ptr_->QueryInterface(__uuidof(U), reinterpret_cast<void**>(ptr.ReleaseAndGetAddressOf()));
        }

    private:

        void AddRef()
        {
            if (ptr_ != nullptr)
                ptr_->AddRef();
        }

        unsigned long Release()
        {
            unsigned long ref = 0;
            
            if (ptr_ != nullptr)
            {
                ref = ptr_->Release();
                ptr_ = nullptr;
            }

            return ref;
        }

        T* ptr_ = nullptr;

};


template <typename T0, typename T1>
bool operator == (const ComPtr<T0>& lhs, const ComPtr<T1>& rhs) throw()
{
    return (lhs.Get() == rhs.Get());
}

template <typename T>
bool operator == (const ComPtr<T>& lhs, decltype(nullptr)) throw()
{
    return (lhs.Get() == nullptr);
}

template <typename T>
bool operator == (decltype(nullptr), const ComPtr<T>& rhs) throw()
{
    return (nullptr == rhs.Get());
}

template <typename T0, typename T1>
bool operator != (const ComPtr<T0>& lhs, const ComPtr<T1>& rhs) throw()
{
    return (lhs.Get() != rhs.Get());
}

template <typename T>
bool operator != (const ComPtr<T>& lhs, decltype(nullptr)) throw()
{
    return (lhs.Get() != nullptr);
}

template <typename T>
bool operator != (decltype(nullptr), const ComPtr<T>& rhs) throw()
{
    return (nullptr != rhs.Get());
}


} // /namespace LLGL


#endif



// ================================================================================
