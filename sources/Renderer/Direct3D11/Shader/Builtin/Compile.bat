REM Compiles all shaders with fxc
REM Use VisualStudio command line tools to run this batch script

fxc /Qstrip_reflect /D TEXTURE_DIM=1 /T cs_5_0 /E CopyTextureFromBuffer /Fh CopyTextureFromBufferCS.Dim1D.cso.inl /Vn g_CopyTextureFromBufferCS_Dim1D CopyTextureFromBuffer.hlsl
fxc /Qstrip_reflect /D TEXTURE_DIM=2 /T cs_5_0 /E CopyTextureFromBuffer /Fh CopyTextureFromBufferCS.Dim2D.cso.inl /Vn g_CopyTextureFromBufferCS_Dim2D CopyTextureFromBuffer.hlsl
fxc /Qstrip_reflect /D TEXTURE_DIM=3 /T cs_5_0 /E CopyTextureFromBuffer /Fh CopyTextureFromBufferCS.Dim3D.cso.inl /Vn g_CopyTextureFromBufferCS_Dim3D CopyTextureFromBuffer.hlsl

fxc /Qstrip_reflect /D TEXTURE_DIM=1 /T cs_5_0 /E CopyBufferFromTexture /Fh CopyBufferFromTextureCS.Dim1D.cso.inl /Vn g_CopyBufferFromTextureCS_Dim1D CopyBufferFromTexture.hlsl
fxc /Qstrip_reflect /D TEXTURE_DIM=2 /T cs_5_0 /E CopyBufferFromTexture /Fh CopyBufferFromTextureCS.Dim2D.cso.inl /Vn g_CopyBufferFromTextureCS_Dim2D CopyBufferFromTexture.hlsl
fxc /Qstrip_reflect /D TEXTURE_DIM=3 /T cs_5_0 /E CopyBufferFromTexture /Fh CopyBufferFromTextureCS.Dim3D.cso.inl /Vn g_CopyBufferFromTextureCS_Dim3D CopyBufferFromTexture.hlsl
