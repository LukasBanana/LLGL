/*
 * D3D11PipelineState.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_PIPELINE_STATE_H
#define LLGL_D3D11_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include "../../../Core/BasicReport.h"


namespace LLGL
{


class D3D11StateManager;

class D3D11PipelineState : public PipelineState
{

    public:

        const Report* GetReport() const override final;

        // Binds this pipeline state to the specified D3D command context.
        virtual void Bind(D3D11StateManager& stateMngr) = 0;

    protected:

        // Writes the report with the specified message and error bit.
        void ResetReport(std::string&& text, bool hasErrors = false);

    private:

        BasicReport report_;

};


} // /namespace LLGL


#endif



// ================================================================================
