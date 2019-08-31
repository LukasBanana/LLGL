/*
 * Resource.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_H
#define LLGL_RESOURCE_H


#include "RenderSystemChild.h"
#include "ResourceFlags.h"


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
