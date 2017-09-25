/*
 * ColorRGBA.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COLOR_RGBA_H
#define LLGL_COLOR_RGBA_H


#include "Color.h"


namespace LLGL
{


/**
\brief RGBA color class with components: r, g, b, and a.
\remarks Color components are default initialized with their maximal value,
i.e. for floating-points, the initial value is 1.0, because this its maximal color value,
but for unsigned-bytes, the initial value is 255.
*/
template <typename T>
class Color<T, 4u>
{
    
    public:
        
        //! Specifies the number of color components.
        static const std::size_t components = 4;

        #ifndef GS_DISABLE_AUTO_INIT
        Color() :
            r { MaxColorValue<T>() },
            g { MaxColorValue<T>() },
            b { MaxColorValue<T>() },
            a { MaxColorValue<T>() }
        {
        }
        #else
        Color() = default;
        #endif

        Color(const Color<T, 4>& rhs) :
            r { rhs.r },
            g { rhs.g },
            b { rhs.b },
            a { rhs.a }
        {
        }

        explicit Color(const T& brightness) :
            r { brightness         },
            g { brightness         },
            b { brightness         },
            a { MaxColorValue<T>() }
        {
        }

        Color(const T& r, const T& g, const T& b) :
            r { r                  },
            g { g                  },
            b { b                  },
            a { MaxColorValue<T>() }
        {
        }

        Color(const T& r, const T& g, const T& b, const T& a) :
            r { r },
            g { g },
            b { b },
            a { a }
        {
        }

        Color(Gs::UninitializeTag)
        {
            // do nothing
        }

        Color<T, 4>& operator += (const Color<T, 4>& rhs)
        {
            r += rhs.r;
            g += rhs.g;
            b += rhs.b;
            a += rhs.a;
            return *this;
        }

        Color<T, 4>& operator -= (const Color<T, 4>& rhs)
        {
            r -= rhs.r;
            g -= rhs.g;
            b -= rhs.b;
            a -= rhs.a;
            return *this;
        }

        Color<T, 4>& operator *= (const Color<T, 4>& rhs)
        {
            r *= rhs.r;
            g *= rhs.g;
            b *= rhs.b;
            a *= rhs.a;
            return *this;
        }

        Color<T, 4>& operator /= (const Color<T, 4>& rhs)
        {
            r /= rhs.r;
            g /= rhs.g;
            b /= rhs.b;
            a /= rhs.a;
            return *this;
        }

        Color<T, 4>& operator *= (const T& rhs)
        {
            r *= rhs;
            g *= rhs;
            b *= rhs;
            a *= rhs;
            return *this;
        }

        Color<T, 4>& operator /= (const T& rhs)
        {
            r /= rhs;
            g /= rhs;
            b /= rhs;
            a /= rhs;
            return *this;
        }

        Color<T, 4> operator - () const
        {
            return Color<T, 4>(-r, -g, -b, -a);
        }

        /**
        \brief Returns the specified color component.
        \param[in] component Specifies the color component index. This must be 0, 1, 2, or 3.
        */
        T& operator [] (std::size_t component)
        {
            GS_ASSERT(component < (Color<T, 4>::components));
            return *((&r) + component);
        }

        /**
        \brief Returns the specified color component.
        \param[in] component Specifies the color component index. This must be 0, 1, 2, or 3.
        */
        const T& operator [] (std::size_t component) const
        {
            GS_ASSERT(component < (Color<T, 4>::components));
            return *((&r) + component);
        }

        //! Returns this RGBA color as RGB color.
        Color<T, 3> ToRGB() const
        {
            return Color<T, 3>(r, g, b);
        }

        /**
        \brief Returns a type casted instance of this color.
        \remarks All color components will be scaled to the range of the new color type.
        \tparam Dst Specifies the destination type.
        */
        template <typename Dst>
        Color<Dst, 4> Cast() const
        {
            return Color<Dst, 4>(
                CastColorValue<Dst>(r),
                CastColorValue<Dst>(g),
                CastColorValue<Dst>(b),
                CastColorValue<Dst>(a)
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

        T r, g, b, a;

};


/* --- Type Alias --- */

template <typename T>
using ColorRGBAT = Color<T, 4>;

using ColorRGBA   = ColorRGBAT<Gs::Real>;
using ColorRGBAb  = ColorRGBAT<bool>;
using ColorRGBAf  = ColorRGBAT<float>;
using ColorRGBAd  = ColorRGBAT<double>;
using ColorRGBAub = ColorRGBAT<std::uint8_t>;


} // /namespace LLGL


#endif



// ================================================================================
