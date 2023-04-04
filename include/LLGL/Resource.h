/*
 * Resource.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RESOURCE_H
#define LLGL_RESOURCE_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/ResourceFlags.h>


namespace LLGL
{


/**
\brief Base class for all hardware resource interfaces.
\see Buffer
\see Texture
\see Sampler
*/
class LLGL_EXPORT Resource : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Resource );

    public:

        /**
        \brief Returns the type of this resource object.
        \remarks This is queried by a virtual function call, so the resource type does not need to be stored per instance.
        \see ResourceType
        \todo (Maybe) replace by IsInstanceOf
        */
        virtual ResourceType GetResourceType() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
