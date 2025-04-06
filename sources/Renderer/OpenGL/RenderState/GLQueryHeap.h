/*
 * GLQueryHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_QUERY_HEAP_H
#define LLGL_GL_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include "../OpenGL.h"
#include <LLGL/Container/Vector.h>


namespace LLGL
{


class GLQueryHeap final : public QueryHeap
{

    public:

        void SetDebugName(const char* name) override;

    public:

        GLQueryHeap(const QueryHeapDescriptor& desc);
        ~GLQueryHeap();

        void Begin(std::uint32_t query);
        void End();

        // Returns the the specified query ID.
        inline GLuint GetID(std::uint32_t query) const
        {
            return ids_[query * GetGroupSize()];
        }

        // Returns the list of hardware query IDs.
        inline const vector<GLuint>& GetIDs() const
        {
            return ids_;
        }

        // Returns the number of IDs for each group of queries.
        inline std::uint32_t GetGroupSize() const
        {
            return groupSize_;
        }

    private:

        vector<GLuint>  ids_;
        std::uint32_t   groupSize_  = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
