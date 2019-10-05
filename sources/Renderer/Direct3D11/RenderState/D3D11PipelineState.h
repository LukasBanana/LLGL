/*
 * D3D11PipelineState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_PIPELINE_STATE_H
#define LLGL_D3D11_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>


namespace LLGL
{


class D3D11StateManager;

class D3D11PipelineState : public PipelineState
{

    public:

        // Binds this pipeline state to the specified D3D command context.
        virtual void Bind(D3D11StateManager& stateMngr) = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
