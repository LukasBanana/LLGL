/*
 * Interface.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_INTERFACE_H
#define LLGL_INTERFACE_H


#include "NonCopyable.h"
#include "InterfaceID.h"


namespace LLGL
{


//! Declares the base functions for all LLGL interfaces.
#define LLGL_DECLARE_INTERFACE(ID)                  \
    public:                                         \
        inline static int GetInterfaceID()          \
        {                                           \
            return (ID);                            \
        }                                           \
        bool IsInstanceOf(int id) const override

//! Implements the base functions for all LLGL interfaces.
#define LLGL_IMPLEMENT_INTERFACE(SELF, BASE)                                \
    bool SELF::IsInstanceOf(int id) const                                   \
    {                                                                       \
        return (id == SELF::GetInterfaceID() || BASE::IsInstanceOf(id));    \
    }


/**
\brief Base class for all interfaces in LLGL.
\see Display
\see RenderSystem
\see RenderSystemChild
\see Surface
\see Timer
*/
class LLGL_EXPORT Interface : public NonCopyable
{

    public:

        /**
        \brief Returns true if this object is an instance of the specified interface.
        \remarks Can be used for a more efficient run-time type information (RTTI) than the default C++ mechanism.
        \remarks This can be used like in the following example:
        \code
        LLGL::Surface* mySurface = ...
        if (mySurface->IsInstanceOf(LLGL::InterfaceID::Window))
        {
            LLGL::Window* myWindow = static_cast<LLGL::Window*>(mySurface);
            ...
        }
        \endcode
        \see LLGL::CastTo
        */
        virtual bool IsInstanceOf(int id) const;

};


} // /namespace LLGL


#endif



// ================================================================================
