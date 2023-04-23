/*
 * ColorRGBA.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COLOR_RGBA_H
#define LLGL_COLOR_RGBA_H


#include <LLGL/Utils/Color.h>


namespace LLGL
{


/**
\brief RGBA color class with components: r, g, b, and a.
\remarks Color components are default initialized with their maximal value,
i.e. for floating-points, the initial value is 1.0, because this its maximal color value,
but for unsigned-bytes, the initial value is 255.
*/
template <typename T>
class LLGL_EXPORT Color<T, 4u>
{

    public:

        //! Specifies the number of color components.
        static constexpr std::size_t components = 4;

        /**
        \brief Constructors all attributes with the default color value.
        \remarks For default color values the 'MaxColorValue' template is used.
        \see MaxColorValue
        */
        Color() :
            r { MaxColorValue<T>() },
            g { MaxColorValue<T>() },
            b { MaxColorValue<T>() },
            a { MaxColorValue<T>() }
        {
        }

        //! Copy constructor.
        Color(const Color<T, 4>& rhs) :
            r { rhs.r },
            g { rhs.g },
            b { rhs.b },
            a { rhs.a }
        {
        }

        //! Constructs all attributes with the specified scalar value.
        explicit Color(const T& scalar) :
            r { scalar },
            g { scalar },
            b { scalar },
            a { scalar }
        {
        }

        /**
        \brief Constructs the RGB attributes with the specified RGB color, and the default value for alpha.
        \remarks For default color values the 'MaxColorValue' template is used.
        \see MaxColorValue
        */
        explicit Color(const Color<T, 3u>& rhs) :
            r { rhs.r              },
            g { rhs.g              },
            b { rhs.b              },
            a { MaxColorValue<T>() }
        {
        }

        /**
        \brief Constructs the RGB attributes with the specified color values r (red), g (green), b (blue), and the default value for alpha.
        \remarks For default color values the 'MaxColorValue' template is used.
        \see MaxColorValue
        */
        Color(const T& r, const T& g, const T& b) :
            r { r                  },
            g { g                  },
            b { b                  },
            a { MaxColorValue<T>() }
        {
        }

        //! Constructs all attributes with the specified color values r (red), g (green), b (blue), a (alpha).
        Color(const T& r, const T& g, const T& b, const T& a) :
            r { r },
            g { g },
            b { b },
            a { a }
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
        Color<T, 4>& operator += (const Color<T, 4>& rhs)
        {
            r += rhs.r;
            g += rhs.g;
            b += rhs.b;
            a += rhs.a;
            return *this;
        }

        //! Substracts the specified color (component wise) from this color.
        Color<T, 4>& operator -= (const Color<T, 4>& rhs)
        {
            r -= rhs.r;
            g -= rhs.g;
            b -= rhs.b;
            a -= rhs.a;
            return *this;
        }

        //! Multiplies the specified color (component wise) with this color.
        Color<T, 4>& operator *= (const Color<T, 4>& rhs)
        {
            r *= rhs.r;
            g *= rhs.g;
            b *= rhs.b;
            a *= rhs.a;
            return *this;
        }

        //! Divides the specified color (component wise) with this color.
        Color<T, 4>& operator /= (const Color<T, 4>& rhs)
        {
            r /= rhs.r;
            g /= rhs.g;
            b /= rhs.b;
            a /= rhs.a;
            return *this;
        }

        //! Multiplies the specified scalar value (component wise) with this color.
        Color<T, 4>& operator *= (const T rhs)
        {
            r *= rhs;
            g *= rhs;
            b *= rhs;
            a *= rhs;
            return *this;
        }

        //! Divides the specified scalar value (component wise) with this color.
        Color<T, 4>& operator /= (const T rhs)
        {
            r /= rhs;
            g /= rhs;
            b /= rhs;
            a /= rhs;
            return *this;
        }

        //! Returns the negation of this color.
        Color<T, 4> operator - () const
        {
            return Color<T, 4>(-r, -g, -b, -a);
        }

        /**
        \brief Returns the specified color component.
        \param[in] component Specifies the color component index. This must be 0, 1, 2, or 3.
        \throws std::out_of_range If the specified component index is out of range (Only if the macro 'LLGL_DEBUG' is defined).
        */
        T& operator [] (std::size_t component)
        {
            #ifdef LLGL_DEBUG
            if (component >= Color<T, 4>::components)
                throw std::out_of_range("color component index out of range (must be 0, 1, 2, or 3)");
            #endif
            return *((&r) + component);
        }

        /**
        \brief Returns the specified color component.
        \param[in] component Specifies the color component index. This must be 0, 1, 2, or 3.
        \throws std::out_of_range If the specified component index is out of range (Only if the macro 'LLGL_DEBUG' is defined).
        */
        const T& operator [] (std::size_t component) const
        {
            #ifdef LLGL_DEBUG
            if (component >= Color<T, 4>::components)
                throw std::out_of_range("color component index out of range (must be 0, 1, 2, or 3)");
            #endif
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
using ColorRGBA = Color<T, 4>;

using ColorRGBAb  = ColorRGBA<bool>;
using ColorRGBAf  = ColorRGBA<float>;
using ColorRGBAd  = ColorRGBA<double>;
using ColorRGBAub = ColorRGBA<std::uint8_t>;


} // /namespace LLGL


#endif



// ================================================================================
