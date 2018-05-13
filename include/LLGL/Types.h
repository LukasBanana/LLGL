/*
 * Types.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TYPES_H
#define LLGL_TYPES_H


#include <Gauss/Vector2.h>


namespace LLGL
{


#if 1

/* ----- Structures ----- */

/**
\brief 2-Dimensional extent structure.
\remarks Used for unsigned integral 2D extents (for sizes in window-space, screen-space, and texture-space).
*/
struct Extent2D
{
    Extent2D() = default;
    Extent2D(const Extent2D&) = default;

    inline Extent2D(std::uint32_t width, std::uint32_t height) :
        width  { width  },
        height { height }
    {
    }

    std::uint32_t width     = 0; //!< Extent X axis, i.e. width.
    std::uint32_t height    = 0; //!< Extent Y axis, i.e. height.
};

/**
\brief 3-Dimensional extent structure.
\remarks Used for unsigned integral 3D extents (for sizes in texture-space).
*/
struct Extent3D
{
    Extent3D() = default;
    Extent3D(const Extent3D&) = default;

    inline Extent3D(std::uint32_t width, std::uint32_t height, std::uint32_t depth) :
        width  { width  },
        height { height },
        depth  { depth  }
    {
    }

    std::uint32_t width     = 0; //!< Extent X axis, i.e. width.
    std::uint32_t height    = 0; //!< Extent Y axis, i.e. height.
    std::uint32_t depth     = 0; //!< Extent Z axis, i.e. depth.
};

/**
\brief 2-Dimensional offset structure.
\remarks Used for signed integral 2D offsets (for coordinates in window-space, screen-space, and texture-space).
*/
struct Offset2D
{
    Offset2D() = default;
    Offset2D(const Offset2D&) = default;

    inline Offset2D(std::int32_t x, std::int32_t y) :
        x { x },
        y { y }
    {
    }

    std::int32_t x = 0; //!< Offset X axis.
    std::int32_t y = 0; //!< Offset Y axis.
};

/**
\brief 3-Dimensional offset structure.
\remarks Used for signed integral 3D offsets (for coordinates in texture-space).
*/
struct Offset3D
{
    Offset3D() = default;
    Offset3D(const Offset3D&) = default;

    inline Offset3D(std::int32_t x, std::int32_t y, std::int32_t z) :
        x { x },
        y { y },
        z { z }
    {
    }

    std::int32_t x = 0; //!< Offset X axis.
    std::int32_t y = 0; //!< Offset Y axis.
    std::int32_t z = 0; //!< Offset Z axis.
};


/* ----- Operators ----- */

//! Returns true if the left hand side offset 'lhs' is equal to the right hand side offset 'rhs'.
inline bool operator == (const Offset2D& lhs, const Offset2D& rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y);
}

//! Returns true if the left hand side offset 'lhs' is unequal to the right hand side offset 'rhs'.
inline bool operator != (const Offset2D& lhs, const Offset2D& rhs)
{
    return !(lhs == rhs);
}

//! Returns true if the left hand side offset 'lhs' is equal to the right hand side offset 'rhs'.
inline bool operator == (const Offset3D& lhs, const Offset3D& rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
}

//! Returns true if the left hand side offset 'lhs' is unequal to the right hand side offset 'rhs'.
inline bool operator != (const Offset3D& lhs, const Offset3D& rhs)
{
    return !(lhs == rhs);
}

//! Returns true if the left hand side extent 'lhs' is equal to the right hand side extent 'rhs'.
inline bool operator == (const Extent2D& lhs, const Extent2D& rhs)
{
    return (lhs.width == rhs.width && lhs.height == rhs.height);
}

//! Returns true if the left hand side extent 'lhs' is unequal to the right hand side extent 'rhs'.
inline bool operator != (const Extent2D& lhs, const Extent2D& rhs)
{
    return !(lhs == rhs);
}

//! Returns true if the left hand side extent 'lhs' is equal to the right hand side extent 'rhs'.
inline bool operator == (const Extent3D& lhs, const Extent3D& rhs)
{
    return (lhs.width == rhs.width && lhs.height == rhs.height && lhs.depth == rhs.depth);
}

//! Returns true if the left hand side extent 'lhs' is unequal to the right hand side extent 'rhs'.
inline bool operator != (const Extent3D& lhs, const Extent3D& rhs)
{
    return !(lhs == rhs);
}

#else //TODO: replace "Point" by "Offset2D" and replace "Size" by "Extent2D"

/**
\brief 2D point (integer).
\todo Rename to "Offset2D" and make it a simple struct.
*/
using Point = Gs::Vector2i;

/**
\brief 2D size (integer).
\todo Rename to "Extent2D" and make it a simple struct.
*/
using Size = Gs::Vector2i;

#endif


} // /namespace LLGL


#endif



// ================================================================================
