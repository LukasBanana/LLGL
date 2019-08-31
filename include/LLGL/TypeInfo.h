/*
 * TypeInfo.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TYPE_INFO_H
#define LLGL_TYPE_INFO_H


#include "Interface.h"


namespace LLGL
{


/**
\brief Returns a constant pointer to this instance of type <code>const T*</code> if it implements the specified interface.
\remarks The typename \c T must be a sub class of Interface and implement the following function:
\code
static InterfaceID GetInterfaceID();
\endcode
\remarks This template can be used like in the following example:
\code
const LLGL::Surface* mySurface = ...
if (const LLGL::Window* myWindow = LLGL::CastTo<LLGL::Window>(mySurface))
{
    ...
}
\endcode
*/
template <typename T>
inline const T* CastTo(const Interface* obj)
{
    return (obj != nullptr && obj->IsInstanceOf(T::GetInterfaceID()) ? static_cast<const T*>(obj) : nullptr);
}

/**
\brief Returns a pointer to this instance of type <code>T*</code> if it implements the specified interface.
\remarks The typename \c T must be a sub class of Interface and implement the following function:
\code
static InterfaceID GetInterfaceID();
\endcode
\remarks This template can be used like in the following example:
\code
LLGL::Surface* mySurface = ...
if (LLGL::Window* myWindow = LLGL::CastTo<LLGL::Window>(mySurface))
{
    ...
}
\endcode
*/
template <typename T>
inline T* CastTo(Interface* obj)
{
    return (obj != nullptr && obj->IsInstanceOf(T::GetInterfaceID()) ? static_cast<T*>(obj) : nullptr);
}

/**
\brief Returns a constant reference to this instance of type <code>const T&</code> if it implements the specified interface.
\remarks The typename \c T must be a sub class of Interface and implement the following function:
\code
static InterfaceID GetInterfaceID();
\endcode
\remarks This template can be used like in the following example:
\code
const LLGL::Surface& mySurface = ...
const LLGL::Window& myWindow = LLGL::CastTo<LLGL::Window>(mySurface);
\endcode
*/
template <typename T>
inline const T& CastTo(const Interface& obj)
{
    if (!obj.IsInstanceOf(T::GetInterfaceID()))
        throw std::bad_cast();
    return static_cast<const T&>(obj);
}

/**
\brief Returns a reference to this instance of type <code>T&</code> if it implements the specified interface.
\remarks The typename \c T must be a sub class of Interface and implement the following function:
\code
static InterfaceID GetInterfaceID();
\endcode
\remarks This template can be used like in the following example:
\code
LLGL::Surface& mySurface = ...
LLGL::Window& myWindow = LLGL::CastTo<LLGL::Window&>(mySurface);
\endcode
*/
template <typename T>
inline T& CastTo(Interface& obj)
{
    if (!obj.IsInstanceOf(T::GetInterfaceID()))
        throw std::bad_cast();
    return static_cast<T&>(obj);
}


} // /namespace LLGL


#endif



// ================================================================================
