/*
 * GLQueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_QUERY_HEAP_H
#define LLGL_GL_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include "../OpenGL.h"
#include <vector>


namespace LLGL
{


class GLQueryHeap final : public QueryHeap
{

    public:

        GLQueryHeap(const QueryHeapDescriptor& desc);
        ~GLQueryHeap();

        void Begin(std::uint32_t query);
        void End(std::uint32_t query);

        // Returns the first ID.
        inline GLuint GetFirstID(std::uint32_t query) const
        {
            return ids_[query * GetGroupSize()];
        }

        // Returns the list of hardware query IDs.
        inline const std::vector<GLuint>& GetIDs() const
        {
            return ids_;
        }

        // Returns the number of IDs for each group of queries.
        inline std::size_t GetGroupSize() const
        {
            return groupSize_;
        }

    private:

        std::vector<GLuint> ids_;
        std::size_t         groupSize_  = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
