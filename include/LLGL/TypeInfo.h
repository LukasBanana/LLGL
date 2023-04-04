/*
 * TypeInfo.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TYPE_INFO_H
#define LLGL_TYPE_INFO_H


#include <LLGL/Interface.h>


namespace LLGL
{


/**
\brief Returns whether the specified object is an instance of the interface specified by template parameter <T>.
\remarks The typename \c T must be a sub class of Interface and implement the following function:
\code
static InterfaceID GetInterfaceID();
\endcode
\see Interface::IsInstanceOf
*/
template <typename T>
inline bool IsInstanceOf(const Interface* obj)
{
    return (obj != nullptr && obj->IsInstanceOf(T::GetInterfaceID()));
}

/**
\brief Const-reference version of IsInstanceOf.
\see IsInstanceOf(const Interface*)
*/
template <typename T>
inline bool IsInstanceOf(const Interface& obj)
{
    return obj.IsInstanceOf(T::GetInterfaceID());
}

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
    return (IsInstanceOf<T>(obj) ? static_cast<const T*>(obj) : nullptr);
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
    return (IsInstanceOf<T>(obj) ? static_cast<T*>(obj) : nullptr);
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
    if (!IsInstanceOf<T>(obj))
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
    if (!IsInstanceOf<T>(obj))
        throw std::bad_cast();
    return static_cast<T&>(obj);
}


} // /namespace LLGL


#endif



// ================================================================================
