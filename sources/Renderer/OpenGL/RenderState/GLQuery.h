/*
 * GLQuery.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_QUERY_H
#define LLGL_GL_QUERY_H


#include <LLGL/Query.h>
#include "../OpenGL.h"
#include <vector>


namespace LLGL
{


class GLQuery final : public Query
{

    public:

        GLQuery(const QueryDescriptor& desc);
        ~GLQuery();

        void Begin();
        void End();

        // Returns the first ID.
        inline GLuint GetFirstID() const
        {
            return ids_.front();
        }

        // Returns the list of hardware query IDs.
        inline const std::vector<GLuint>& GetIDs() const
        {
            return ids_;
        }

    private:

        std::vector<GLuint> ids_;

};


} // /namespace LLGL


#endif



// ================================================================================
