/*
 * ColorRGBA.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

        /**
        \brief Returns a type casted instance of this color.
        \remarks All color components will be scaled to the range of the new color type.
        \tparam C Specifies the static cast type.
        */
        template <typename C>
        Color<C, 4> Cast() const
        {
            if (MaxColorValue<T>() != MaxColorValue<C>())
            {
                /* Transform color components */
                auto oldRange = static_cast<double>(MaxColorValue<T>());
                auto newRange = static_cast<double>(MaxColorValue<C>());

                auto newR = static_cast<double>(r) * newRange / oldRange;
                auto newG = static_cast<double>(g) * newRange / oldRange;
                auto newB = static_cast<double>(b) * newRange / oldRange;
                auto newA = static_cast<double>(a) * newRange / oldRange;

                return Color<C, 4>(
                    static_cast<C>(newR),
                    static_cast<C>(newG),
                    static_cast<C>(newB),
                    static_cast<C>(newA)
                );
            }
            else
            {
                /* Cast the color untransformed */
                return Color<C, 4>(
                    static_cast<C>(r),
                    static_cast<C>(g),
                    static_cast<C>(b),
                    static_cast<C>(a)
                );
            }
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
using ColorRGBAub = ColorRGBAT<unsigned char>;


} // /namespace LLGL


#endif



// ================================================================================
