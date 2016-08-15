/*
 * RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_CONTEXT_H__
#define __LLGL_RENDER_CONTEXT_H__


#include "Export.h"
#include "Window.h"
#include "RenderContextDescriptor.h"
#include "RenderContextFlags.h"
#include "ColorRGBA.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "ShaderProgram.h"
#include "Texture.h"

#include <Gauss/Vector3.h>
#include <string>
#include <map>


namespace LLGL
{


/**
\brief Enumeration of all renderer info entries.
\see RenderContext::QueryRendererInfo
*/
enum class RendererInfo
{
    Version,
    Vendor,
    Hardware,
    ShadingLanguageVersion,
};


//! Render context interface.
class LLGL_EXPORT RenderContext
{

    public:

        /* ----- Common ----- */

        RenderContext(const RenderContext&) = delete;
        RenderContext& operator = (const RenderContext&) = delete;

        virtual ~RenderContext();

        //! Returns all available renderer information.
        virtual std::map<RendererInfo, std::string> QueryRendererInfo() const = 0;

        //! Returns the rendering capabilities.
        virtual RenderingCaps QueryRenderingCaps() const = 0;

        //! Returns the highest version of the supported shading language.
        virtual ShadingLanguage QueryShadingLanguage() const = 0;

        //! Presents the current frame on the screen.
        virtual void Present() = 0;

        //! Returns the window which is used to draw all content.
        inline Window& GetWindow() const
        {
            return *window_;
        }

        /* ----- Configuration ----- */

        virtual void SetClearColor(const ColorRGBAf& color) = 0;
        virtual void SetClearDepth(float depth) = 0;
        virtual void SetClearStencil(int stencil) = 0;

        virtual void ClearBuffers(long flags) = 0;

        virtual void SetDrawMode(const DrawMode drawMode) = 0;

        /* ----- Hardware buffers ------ */

        virtual void BindVertexBuffer(VertexBuffer& vertexBuffer) = 0;
        virtual void UnbindVertexBuffer() = 0;

        virtual void BindIndexBuffer(IndexBuffer& indexBuffer) = 0;
        virtual void UnbindIndexBuffer() = 0;

        virtual void BindConstantBuffer(ConstantBuffer& constantBuffer, unsigned int index) = 0;
        virtual void UnbindConstantBuffer(unsigned int index) = 0;

        /* ----- Textures ----- */

        virtual void BindTexture(Texture& texture, unsigned int layer) = 0;
        virtual void UnbindTexture(unsigned int layer) = 0;

        /**
        \brief Generates the MIP ("Multum in Parvo") maps for the specified texture.
        \see https://developer.valvesoftware.com/wiki/MIP_Mapping
        */
        virtual void GenerateMips(Texture& texture) = 0;

        /* ----- Shader ----- */

        virtual void BindShaderProgram(ShaderProgram& shaderProgram) = 0;
        virtual void UnbindShaderProgram() = 0;

        /* ----- Pipeline states ----- */

        //virtual void BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline) = 0;
        //virtual void BindComputePipeline(ComputePipeline& computePipeline) = 0;

        /* ----- Drawing ----- */

        virtual void Draw(unsigned int numVertices, unsigned int firstVertex) = 0;

        virtual void DrawIndexed(unsigned int numVertices) = 0;
        virtual void DrawIndexed(unsigned int numVertices, int indexOffset) = 0;

        virtual void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) = 0;
        virtual void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) = 0;

        virtual void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances) = 0;
        virtual void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset) = 0;
        virtual void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset, unsigned int instanceOffset) = 0;

        virtual void DispatchCompute(const Gs::Vector3ui& threadGroupSize) = 0;

    protected:

        RenderContext() = default;

        void SetWindow(const std::shared_ptr<Window>& window, VideoModeDescriptor& videoModeDesc, const void* windowContext);

    private:

        std::shared_ptr<Window> window_;

};


} // /namespace LLGL


#endif



// ================================================================================
