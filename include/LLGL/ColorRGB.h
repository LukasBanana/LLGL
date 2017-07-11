/*
 * ColorRGB.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COLOR_RGB_H
#define LLGL_COLOR_RGB_H


#include "Color.h"


namespace LLGL
{


/**
\brief RGB color class with components: r, g, and b.
\remarks Color components are default initialized with their maximal value,
i.e. for floating-points, the initial value is 1.0, because this its maximal color value,
but for unsigned-bytes, the initial value is 255.
*/
template <typename T>
class Color<T, 3u>
{
    
    public:
        
        //! Specifies the number of color components.
        static const std::size_t components = 3;

        #ifndef GS_DISABLE_AUTO_INIT
        Color() :
            r { MaxColorValue<T>() },
            g { MaxColorValue<T>() },
            b { MaxColorValue<T>() }
        {
        }
        #else
        Color() = default;
        #endif

        Color(const Color<T, 3>& rhs) :
            r { rhs.r },
            g { rhs.g },
            b { rhs.b }
        {
        }

        explicit Color(const T& scalar) :
            r { scalar },
            g { scalar },
            b { scalar }
        {
        }

        Color(const T& r, const T& g, const T& b) :
            r { r },
            g { g },
            b { b }
        {
        }

        Color(Gs::UninitializeTag)
        {
            // do nothing
        }

        Color<T, 3>& operator += (const Color<T, 3>& rhs)
        {
            r += rhs.r;
            g += rhs.g;
            b += rhs.b;
            return *this;
        }

        Color<T, 3>& operator -= (const Color<T, 3>& rhs)
        {
            r -= rhs.r;
            g -= rhs.g;
            b -= rhs.b;
            return *this;
        }

        Color<T, 3>& operator *= (const Color<T, 3>& rhs)
        {
            r *= rhs.r;
            g *= rhs.g;
            b *= rhs.b;
            return *this;
        }

        Color<T, 3>& operator /= (const Color<T, 3>& rhs)
        {
            r /= rhs.r;
            g /= rhs.g;
            b /= rhs.b;
            return *this;
        }

        Color<T, 3>& operator *= (const T& rhs)
        {
            r *= rhs;
            g *= rhs;
            b *= rhs;
            return *this;
        }

        Color<T, 3>& operator /= (const T& rhs)
        {
            r /= rhs;
            g /= rhs;
            b /= rhs;
            return *this;
        }

        Color<T, 3> operator - () const
        {
            return Color<T, 3>(-r, -g, -b);
        }

        /**
        \brief Returns the specified color component.
        \param[in] component Specifies the color component index. This must be 0, 1, or 2.
        */
        T& operator [] (std::size_t component)
        {
            GS_ASSERT(component < (Color<T, 3>::components));
            return *((&r) + component);
        }

        /**
        \brief Returns the specified color component.
        \param[in] component Specifies the color component index. This must be 0, 1, or 2.
        */
        const T& operator [] (std::size_t component) const
        {
            GS_ASSERT(component < (Color<T, 3>::components));
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
using ColorRGBT = Color<T, 3>;

using ColorRGB      = ColorRGBT<Gs::Real>;
using ColorRGBb     = ColorRGBT<bool>;
using ColorRGBf     = ColorRGBT<float>;
using ColorRGBd     = ColorRGBT<double>;
using ColorRGBub    = ColorRGBT<unsigned char>;


} // /namespace LLGL


#endif



// ================================================================================
