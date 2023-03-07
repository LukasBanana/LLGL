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


class PipelineLayout;
class D3D11StateManager;
class D3D11PipelineLayout;

class D3D11PipelineState : public PipelineState
{

    public:

        const Report* GetReport() const override final;

        // Binds this pipeline state to the specified D3D command context.
        virtual void Bind(D3D11StateManager& stateMngr) = 0;

        // Returns true if this is a graphics PSO.
        inline bool IsGraphicsPSO() const
        {
            return isGraphicsPSO_;
        }

        // Returns the pipeline layout this PSO was created with. May also be null.
        inline const D3D11PipelineLayout* GetPipelineLayout() const
        {
            return pipelineLayout_;
        }

    protected:

        D3D11PipelineState(bool isGraphicsPSO, const PipelineLayout* pipelineLayout);

        // Writes the report with the specified message and error bit.
        void ResetReport(std::string&& text, bool hasErrors = false);

    private:

        const bool                  isGraphicsPSO_  = false;
        const D3D11PipelineLayout*  pipelineLayout_ = nullptr;
        BasicReport                 report_;

};


} // /namespace LLGL


#endif



// ================================================================================
