/*
 * Color.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COLOR_H
#define LLGL_COLOR_H


#include <Gauss/Real.h>
#include <Gauss/Assert.h>
#include <Gauss/Tags.h>
#include <Gauss/Equals.h>

#include <algorithm>


namespace LLGL
{



/* --- Global functions --- */

//! Returns the maximal color value for the data type T. By default 1.
template <typename T>
inline T MaxColorValue()
{
    return T(1);
}

//! Specialized version. For unsigned 8-bit integers, the return value is 255.
template <>
inline unsigned char MaxColorValue<unsigned char>()
{
    return 255;
}

//! Specialized version. For booleans, the return value is true.
template <>
inline bool MaxColorValue<bool>()
{
    return true;
}


/* --- Color class --- */

/**
\brief Base color class with N components.
\tparam T Specifies the data type of the vector components.
This should be a primitive data type such as float, double, int etc.
\tparam N Specifies the number of components. There are specialized templates for N = 3, and 4.
*/
template <typename T, std::size_t N>
class Color
{
    
    public:
        
        //! Specifies the number of vector components.
        static const std::size_t components = N;

        #ifndef GS_DISABLE_AUTO_INIT
        Color()
        {
            std::fill(std::begin(v_), std::end(v_), MaxColorValue<T>());
        }
        #else
        Color() = default;
        #endif

        Color(const Color<T, N>& rhs)
        {
            std::copy(std::begin(rhs.v_), std::end(rhs.v_), v_);
        }

        Color(Gs::UninitializeTag)
        {
            // do nothing
        }

        Color<T, N>& operator += (const Color<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] += rhs[i];
            return *this;
        }

        Color<T, N>& operator -= (const Color<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] -= rhs[i];
            return *this;
        }

        Color<T, N>& operator *= (const Color<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] *= rhs[i];
            return *this;
        }

        Color<T, N>& operator /= (const Color<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] /= rhs[i];
            return *this;
        }

        Color<T, N>& operator *= (const T& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] *= rhs;
            return *this;
        }

        Color<T, N>& operator /= (const T& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] /= rhs;
            return *this;
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be in the range [0, N).
        */
        T& operator [] (std::size_t component)
        {
            GS_ASSERT(component < N);
            return v_[component];
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be in the range [0, N).
        */
        const T& operator [] (std::size_t component) const
        {
            GS_ASSERT(component < N);
            return v_[component];
        }

        Color<T, N> operator - () const
        {
            auto result = *this;
            for (std::size_t i = 0; i < N; ++i)
                result[i] = -result[i];
            return result;
        }

        /**
        Returns a type casted instance of this vector.
        \tparam C Specifies the static cast type.
        */
        template <typename C>
        Color<C, N> Cast() const
        {
            Color<C, N> result(Gs::UninitializeTag{});

            for (std::size_t i = 0; i < N; ++i)
                result[i] = static_cast<C>(v_[i]);

            return result;
        }

        //! Returns a pointer to the first element of this vector.
        T* Ptr()
        {
            return v_;
        }

        //! Returns a constant pointer to the first element of this vector.
        const T* Ptr() const
        {
            return v_;
        }

    private:
        
        T v_[N];

};


/* --- Global operators --- */

template <typename T, std::size_t N>
Color<T, N> operator + (const Color<T, N>& lhs, const Color<T, N>& rhs)
{
    auto result = lhs;
    result += rhs;
    return result;
}

template <typename T, std::size_t N>
Color<T, N> operator - (const Color<T, N>& lhs, const Color<T, N>& rhs)
{
    auto result = lhs;
    result -= rhs;
    return result;
}

template <typename T, std::size_t N>
Color<T, N> operator * (const Color<T, N>& lhs, const Color<T, N>& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template <typename T, std::size_t N>
Color<T, N> operator / (const Color<T, N>& lhs, const Color<T, N>& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template <typename T, std::size_t N>
Color<T, N> operator * (const Color<T, N>& lhs, const T& rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template <typename T, std::size_t N>
Color<T, N> operator * (const T& lhs, const Color<T, N>& rhs)
{
    auto result = rhs;
    result *= lhs;
    return result;
}

template <typename T, std::size_t N>
Color<T, N> operator / (const Color<T, N>& lhs, const T& rhs)
{
    auto result = lhs;
    result /= rhs;
    return result;
}

template <typename T, std::size_t N>
bool operator == (const Color<T, N>& lhs, const Color<T, N>& rhs)
{
    for (std::size_t i = 0; i < N; ++i)
    {
        if (!Gs::Equals(lhs[i], rhs[i]))
            return false;
    }
    return true;
}

template <typename T, std::size_t N>
bool operator != (const Color<T, N>& lhs, const Color<T, N>& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL


#endif



// ================================================================================
