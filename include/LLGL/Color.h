/*
 * Color.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COLOR_H
#define LLGL_COLOR_H


#include <LLGL/Export.h>
#include <LLGL/Tags.h>
#include <algorithm>
#include <type_traits>
#include <cstdint>
#include <stdexcept>


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
inline std::uint8_t MaxColorValue<std::uint8_t>()
{
    return 255;
}

//! Specialized version. For booleans, the return value is true.
template <>
inline bool MaxColorValue<bool>()
{
    return true;
}

/**
\brief Casts the specified color value and transforms it from the source data type range to the destination data type range.
\see MaxColorValue
*/
template <typename Dst, typename Src>
inline Dst CastColorValue(const Src& value)
{
    /* Use double as intermediate type, if either source or destination type is double */
    using T = typename std::conditional
        <
            (std::is_same<Src, double>::value || std::is_same<Dst, double>::value),
            double,
            float
        >
        ::type;

    /* Get data type ranges */
    const auto srcRange = static_cast<T>(MaxColorValue<Src>());
    const auto dstRange = static_cast<T>(MaxColorValue<Dst>());

    /* Transform input value into new range */
    return static_cast<Dst>(static_cast<T>(value) * dstRange / srcRange);
}

//! Specialized template which merely passes the input value as output.
template <>
inline bool CastColorValue<bool, bool>(const bool& value)
{
    return value;
}

//! Specialized template which merely passes the input value as output.
template <>
inline float CastColorValue<float, float>(const float& value)
{
    return value;
}

//! Specialized template which merely passes the input value as output.
template <>
inline double CastColorValue<double, double>(const double& value)
{
    return value;
}

//! Specialized template which merely passes the input value as output.
template <>
inline std::uint8_t CastColorValue<std::uint8_t, std::uint8_t>(const std::uint8_t& value)
{
    return value;
}


/* --- Color class --- */

/**
\brief Base color class with N components.
\tparam T Specifies the data type of the vector components.
This should be a primitive data type such as float, double, int etc.
\tparam N Specifies the number of components. There are specialized templates for N = 3, and 4.
*/
template <typename T, std::size_t N>
class LLGL_EXPORT Color
{

    public:

        //! Specifies the number of vector components.
        static constexpr std::size_t components = N;

        /**
        \brief Constructors all attributes with the default color value.
        \remarks For default color values the 'MaxColorValue' template is used.
        \see MaxColorValue
        */
        Color()
        {
            std::fill(std::begin(v_), std::end(v_), MaxColorValue<T>());
        }

        //! Copy constructor.
        Color(const Color<T, N>& rhs)
        {
            std::copy(std::begin(rhs.v_), std::end(rhs.v_), v_);
        }

        //! Constructs all attributes with the specified scalar value.
        explicit Color(const T& scalar)
        {
            std::fill(std::begin(v_), std::end(v_), scalar);
        }

        /**
        \brief Explicitly uninitialized constructor. All attributes are uninitialized!
        \remarks Only use this constructor when you want to allocate a large amount of color elements that are being initialized later.
        */
        Color(UninitializeTag)
        {
            // do nothing
        }

        //! Adds the specified color (component wise) to this color.
        Color<T, N>& operator += (const Color<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] += rhs[i];
            return *this;
        }

        //! Substracts the specified color (component wise) from this color.
        Color<T, N>& operator -= (const Color<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] -= rhs[i];
            return *this;
        }

        //! Multiplies the specified color (component wise) with this color.
        Color<T, N>& operator *= (const Color<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] *= rhs[i];
            return *this;
        }

        //! Divides the specified color (component wise) with this color.
        Color<T, N>& operator /= (const Color<T, N>& rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] /= rhs[i];
            return *this;
        }

        //! Multiplies the specified scalar value (component wise) with this color.
        Color<T, N>& operator *= (const T rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] *= rhs;
            return *this;
        }

        //! Divides the specified scalar value (component wise) with this color.
        Color<T, N>& operator /= (const T rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                v_[i] /= rhs;
            return *this;
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be in the range [0, N).
        \throws std::out_of_range If the specified component index is out of range (Only if the macro 'LLGL_DEBUG' is defined).
        */
        T& operator [] (std::size_t component)
        {
            #ifdef LLGL_DEBUG
            if (component >= N)
                throw std::out_of_range("color component index out of range");
            #endif
            return v_[component];
        }

        /**
        \brief Returns the specified vector component.
        \param[in] component Specifies the vector component index. This must be in the range [0, N).
        \throws std::out_of_range If the specified component index is out of range (Only if the macro 'LLGL_DEBUG' is defined).
        */
        const T& operator [] (std::size_t component) const
        {
            #ifdef LLGL_DEBUG
            if (component >= N)
                throw std::out_of_range("color component index out of range");
            #endif
            return v_[component];
        }

        //! Returns the negation of this color.
        Color<T, N> operator - () const
        {
            auto result = *this;
            for (std::size_t i = 0; i < N; ++i)
                result[i] = -result[i];
            return result;
        }

        /**
        \brief Returns a type casted instance of this color.
        \remarks All color components will be scaled to the range of the new color type.
        \tparam Dst Specifies the destination type.
        */
        template <typename Dst>
        Color<Dst, N> Cast() const
        {
            Color<Dst, N> result { UninitializeTag{} };

            for (std::size_t i = 0; i < N; ++i)
                result[i] = CastColorValue<Dst>(v_[i]);

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
Color<T, N> operator / (const T& lhs, const Color<T, N>& rhs)
{
    auto result = Color<T, N>(lhs);
    result /= rhs;
    return result;
}

/**
\brief Returns true if all components of both colors 'lhs' and 'rhs' are equal.
\remarks The comparison uses the 'operator ==' of the underlying component type.
Note that this comparison is quite limited for floating-point types, due to precision issues.
*/
template <typename T, std::size_t N>
bool operator == (const Color<T, N>& lhs, const Color<T, N>& rhs)
{
    for (std::size_t i = 0; i < N; ++i)
    {
        if (!(lhs[i] == rhs[i]))
            return false;
    }
    return true;
}

/**
\brief Returns true if any component of both colors 'lhs' and 'rhs' are unequal.
\remarks The comparison uses the 'operator ==' of the underlying component type.
Note that this comparison is quite limited for floating-point types, due to precision issues.
*/
template <typename T, std::size_t N>
bool operator != (const Color<T, N>& lhs, const Color<T, N>& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL


#endif



// ================================================================================
