/*
 * GLStaticAssertions.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../StaticAssertions.h"
#include "RenderState/GLState.h"
#include "RenderState/GLContextState.h"
#include "Command/GLCommand.h"


namespace LLGL
{


// POD structures
LLGL_ASSERT_POD_TYPE( GLViewport );
LLGL_ASSERT_POD_TYPE( GLDepthRange );
LLGL_ASSERT_POD_TYPE( GLScissor );

// GLState.h
LLGL_ASSERT_STDLAYOUT_STRUCT( GLRenderState );

// GLContextState.h
LLGL_ASSERT_STDLAYOUT_STRUCT( GLContextState );

// GLCommand.h
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBufferSubData );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdCopyBufferSubData );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdClearBufferData );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdClearBufferSubData );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdCopyImageSubData ); //NOTE: non-POD struct
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdCopyImageBuffer ); //NOTE: non-POD struct
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdCopyFramebufferSubData ); //NOTE: non-POD struct
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdGenerateMipmap );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdGenerateMipmapSubresource );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdExecute );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdViewport );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdViewportArray );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdScissor );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdScissorArray );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdClearColor );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdClearDepth );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdClearStencil );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdClear );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdClearAttachmentsWithRenderPass );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdClearBuffers );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdResolveRenderTarget );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindVertexArray );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindElementArrayBufferToVAO );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindBufferBase );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindBuffersBase );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBeginBufferXfb );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBeginTransformFeedback );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBeginTransformFeedbackNV );
//LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdEndTransformFeedback ); // Unused
//LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdEndTransformFeedbackNV ); // Unused
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindResourceHeap );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindRenderTarget );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindPipelineState );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdSetBlendColor );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdSetStencilRef );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdSetUniform );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBeginQuery );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdEndQuery );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBeginConditionalRender );
//LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdEndConditionalRender ); // Unused
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawArrays );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawArraysInstanced );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawArraysInstancedBaseInstance );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawArraysIndirect );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawElements );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawElementsBaseVertex );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawElementsInstanced );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawElementsInstancedBaseVertex );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawElementsInstancedBaseVertexBaseInstance );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawElementsIndirect );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdMultiDrawArraysIndirect );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdMultiDrawElementsIndirect );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawTransformFeedback );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDrawEmulatedTransformFeedback );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDispatchCompute );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdDispatchComputeIndirect );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindTexture );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindImageTexture );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindSampler );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdBindEmulatedSampler );
LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdPushDebugGroup );
//LLGL_ASSERT_STDLAYOUT_STRUCT( GLCmdPopDebugGroup ); // Unused

// Structs used as payload
LLGL_ASSERT_STDLAYOUT_STRUCT( ClearValue ); // Payload for GLCmdClearAttachmentsWithRenderPass
LLGL_ASSERT_STDLAYOUT_STRUCT( AttachmentClear ); // Payload for GLCmdClearBuffers


} // /namespace LLGL



// ================================================================================
