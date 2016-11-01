/*
 * GLQuery.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_QUERY_H
#define LLGL_GL_QUERY_H


#include <LLGL/Query.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLQuery : public Query
{

    public:

        GLQuery(const QueryDescriptor& desc);
        ~GLQuery();

        //! Returns the query target.
        inline GLenum GetTarget() const
        {
            return target_;
        }

        //! Returns the hardware query ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLenum target_  = 0;
        GLuint id_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
