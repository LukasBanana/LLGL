/*
 * GLStaticAssertions.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../StaticAssertions.h"
#include "RenderState/GLState.h"
#include "Command/GLCommand.h"


namespace LLGL
{


// GLState.h
LLGL_ASSERT_POD_STRUCT( GLViewport );
LLGL_ASSERT_POD_STRUCT( GLDepthRange );
LLGL_ASSERT_POD_STRUCT( GLScissor );
LLGL_ASSERT_POD_STRUCT( GLRenderState );
LLGL_ASSERT_POD_STRUCT( GLClearValue );

// GLCommand.h
LLGL_ASSERT_POD_STRUCT( GLCmdBufferSubData );
LLGL_ASSERT_POD_STRUCT( GLCmdCopyBufferSubData );
//LLGL_ASSERT_POD_STRUCT( GLCmdCopyImageSubData ); //TODO: must be converted into a POD struct!
LLGL_ASSERT_POD_STRUCT( GLCmdGenerateMipmap );
LLGL_ASSERT_POD_STRUCT( GLCmdGenerateMipmapSubresource );
LLGL_ASSERT_POD_STRUCT( GLCmdExecute );
//LLGL_ASSERT_POD_STRUCT( GLCmdSetAPIDepState ); //TODO: must be converted into a POD struct!
LLGL_ASSERT_POD_STRUCT( GLCmdViewport );
LLGL_ASSERT_POD_STRUCT( GLCmdViewportArray );
LLGL_ASSERT_POD_STRUCT( GLCmdScissor );
LLGL_ASSERT_POD_STRUCT( GLCmdScissorArray );
LLGL_ASSERT_POD_STRUCT( GLCmdClearColor );
LLGL_ASSERT_POD_STRUCT( GLCmdClearDepth );
LLGL_ASSERT_POD_STRUCT( GLCmdClearStencil );
LLGL_ASSERT_POD_STRUCT( GLCmdClear );
LLGL_ASSERT_POD_STRUCT( GLCmdClearBuffers );
LLGL_ASSERT_POD_STRUCT( GLCmdBindVertexArray );
LLGL_ASSERT_POD_STRUCT( GLCmdBindGL2XVertexArray );
LLGL_ASSERT_POD_STRUCT( GLCmdBindElementArrayBufferToVAO );
LLGL_ASSERT_POD_STRUCT( GLCmdBindBufferBase );
LLGL_ASSERT_POD_STRUCT( GLCmdBindBuffersBase );
LLGL_ASSERT_POD_STRUCT( GLCmdBeginTransformFeedback );
LLGL_ASSERT_POD_STRUCT( GLCmdBeginTransformFeedbackNV );
//LLGL_ASSERT_POD_STRUCT( GLCmdEndTransformFeedback ); // Unused
//LLGL_ASSERT_POD_STRUCT( GLCmdEndTransformFeedbackNV ); // Unused
LLGL_ASSERT_POD_STRUCT( GLCmdBindResourceHeap );
LLGL_ASSERT_POD_STRUCT( GLCmdBindRenderPass );
LLGL_ASSERT_POD_STRUCT( GLCmdBindGraphicsPipeline );
LLGL_ASSERT_POD_STRUCT( GLCmdBindComputePipeline );
LLGL_ASSERT_POD_STRUCT( GLCmdSetUniforms );
LLGL_ASSERT_POD_STRUCT( GLCmdBeginQuery );
LLGL_ASSERT_POD_STRUCT( GLCmdEndQuery );
LLGL_ASSERT_POD_STRUCT( GLCmdBeginConditionalRender );
//LLGL_ASSERT_POD_STRUCT( GLCmdEndConditionalRender ); // Unused
LLGL_ASSERT_POD_STRUCT( GLCmdDrawArrays );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawArraysInstanced );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawArraysInstancedBaseInstance );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawArraysIndirect );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawElements );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawElementsBaseVertex );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawElementsInstanced );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawElementsInstancedBaseVertex );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawElementsInstancedBaseVertexBaseInstance );
LLGL_ASSERT_POD_STRUCT( GLCmdDrawElementsIndirect );
LLGL_ASSERT_POD_STRUCT( GLCmdMultiDrawArraysIndirect );
LLGL_ASSERT_POD_STRUCT( GLCmdMultiDrawElementsIndirect );
LLGL_ASSERT_POD_STRUCT( GLCmdDispatchCompute );
LLGL_ASSERT_POD_STRUCT( GLCmdDispatchComputeIndirect );
LLGL_ASSERT_POD_STRUCT( GLCmdBindTexture );
LLGL_ASSERT_POD_STRUCT( GLCmdBindSampler );
LLGL_ASSERT_POD_STRUCT( GLCmdUnbindResources );
LLGL_ASSERT_POD_STRUCT( GLCmdPushDebugGroup );
//LLGL_ASSERT_POD_STRUCT( GLCmdPopDebugGroup ); // Unused


} // /namespace LLGL



// ================================================================================
