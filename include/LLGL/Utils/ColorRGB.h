/*
 * ColorRGB.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COLOR_RGB_H
#define LLGL_COLOR_RGB_H


#include <LLGL/Utils/Color.h>


namespace LLGL
{


/**
\brief RGB color class with components: r, g, and b.
\remarks Color components are default initialized with their maximal value,
i.e. for floating-points, the initial value is 1.0, because this its maximal color value,
but for unsigned-bytes, the initial value is 255.
*/
template <typename T>
class LLGL_EXPORT Color<T, 3u>
{

    public:

        //! Specifies the number of color components.
        static constexpr std::size_t components = 3;

        /**
        \brief Constructors all attributes with the default color value.
        \remarks For default color values the 'MaxColorValue' template is used.
        \see MaxColorValue
        */
        Color() :
            r { MaxColorValue<T>() },
            g { MaxColorValue<T>() },
            b { MaxColorValue<T>() }
        {
        }

        //! Copy constructor.
        Color(const Color<T, 3>& rhs) :
            r { rhs.r },
            g { rhs.g },
            b { rhs.b }
        {
        }

        //! Constructs all attributes with the specified scalar value.
        explicit Color(const T& scalar) :
            r { scalar },
            g { scalar },
            b { scalar }
        {
        }

        //! Constructs all attributes with the specified color values r (red), g (green), b (blue).
        Color(const T& r, const T& g, const T& b) :
            r { r },
            g { g },
            b { b }
        {
        }

        /**
        \brief Explicitly uninitialized constructor. All attributes are uninitialized!
        \remarks Only use this constructor when you want to allocate a large amount of color elements that are being initialized later.
        */
        explicit Color(UninitializeTag)
        {
            // do nothing
        }

        //! Adds the specified color (component wise) to this color.
        Color<T, 3>& operator += (const Color<T, 3>& rhs)
        {
            r += rhs.r;
            g += rhs.g;
            b += rhs.b;
            return *this;
        }

        //! Substracts the specified color (component wise) from this color.
        Color<T, 3>& operator -= (const Color<T, 3>& rhs)
        {
            r -= rhs.r;
            g -= rhs.g;
            b -= rhs.b;
            return *this;
        }

        //! Multiplies the specified color (component wise) with this color.
        Color<T, 3>& operator *= (const Color<T, 3>& rhs)
        {
            r *= rhs.r;
            g *= rhs.g;
            b *= rhs.b;
            return *this;
        }

        //! Divides the specified color (component wise) with this color.
        Color<T, 3>& operator /= (const Color<T, 3>& rhs)
        {
            r /= rhs.r;
            g /= rhs.g;
            b /= rhs.b;
            return *this;
        }

        //! Multiplies the specified scalar value (component wise) with this color.
        Color<T, 3>& operator *= (const T rhs)
        {
            r *= rhs;
            g *= rhs;
            b *= rhs;
            return *this;
        }

        //! Divides the specified scalar value (component wise) with this color.
        Color<T, 3>& operator /= (const T rhs)
        {
            r /= rhs;
            g /= rhs;
            b /= rhs;
            return *this;
        }

        //! Returns the negation of this color.
        Color<T, 3> operator - () const
        {
            return Color<T, 3>(-r, -g, -b);
        }

        /**
        \brief Returns the specified color component.
        \param[in] component Specifies the color component index. This must be 0, 1, or 2.
        \throws std::out_of_range If the specified component index is out of range (Only if the macro 'LLGL_DEBUG' is defined).
        */
        T& operator [] (std::size_t component)
        {
            #ifdef LLGL_DEBUG
            if (component >= Color<T, 3>::components)
                throw std::out_of_range("color component index out of range (must be 0, 1, or 2)");
            #endif
            return *((&r) + component);
        }

        /**
        \brief Returns the specified color component.
        \param[in] component Specifies the color component index. This must be 0, 1, or 2.
        \throws std::out_of_range If the specified component index is out of range (Only if the macro 'LLGL_DEBUG' is defined).
        */
        const T& operator [] (std::size_t component) const
        {
            #ifdef LLGL_DEBUG
            if (component >= Color<T, 3>::components)
                throw std::out_of_range("color component index out of range (must be 0, 1, or 2)");
            #endif
            return *((&r) + component);
        }

        //! Returns this RGB color as RGBA color.
        Color<T, 4> ToRGBA() const
        {
            return Color<T, 4>(r, g, b);
        }

        /**
        \brief Returns a type casted instance of this color.
        \remarks All color components will be scaled to the range of the new color type.
        \tparam Dst Specifies the destination type.
        */
        template <typename Dst>
        Color<Dst, 3> Cast() const
        {
            return Color<Dst, 3>(
                CastColorValue<Dst>(r),
                CastColorValue<Dst>(g),
                CastColorValue<Dst>(b)
            );
        }

        //! Returns a pointer to the first element of this color.
        T* Ptr()
        {
            return &r;
        }

        //! Returns a constant pointer to the first element of this color.
        const T* Ptr() const
        {
            return &r;
        }

        T r, g, b;

};


/* --- Type Alias --- */

template <typename T>
using ColorRGB = Color<T, 3>;

using ColorRGBb     = ColorRGB<bool>;
using ColorRGBf     = ColorRGB<float>;
using ColorRGBd     = ColorRGB<double>;
using ColorRGBub    = ColorRGB<std::uint8_t>;


} // /namespace LLGL


#endif



// ================================================================================
