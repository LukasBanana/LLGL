REM Compiles all shaders with fxc
REM Use VisualStudio command line tools to run this batch script

fxc /D TEXTURE_DIM=1 /T cs_5_0 /E CopyTextureFromBuffer /Fh CopyTextureFromBufferCS.Dim1D.inc /Vn CopyTextureFromBufferCS_Dim1D CopyTextureFromBuffer.hlsl
fxc /D TEXTURE_DIM=2 /T cs_5_0 /E CopyTextureFromBuffer /Fh CopyTextureFromBufferCS.Dim2D.inc /Vn CopyTextureFromBufferCS_Dim2D CopyTextureFromBuffer.hlsl
fxc /D TEXTURE_DIM=3 /T cs_5_0 /E CopyTextureFromBuffer /Fh CopyTextureFromBufferCS.Dim3D.inc /Vn CopyTextureFromBufferCS_Dim3D CopyTextureFromBuffer.hlsl

fxc /D TEXTURE_DIM=1 /T cs_5_0 /E CopyBufferFromTexture /Fh CopyBufferFromTextureCS.Dim1D.inc /Vn CopyBufferFromTextureCS_Dim1D CopyBufferFromTexture.hlsl
fxc /D TEXTURE_DIM=2 /T cs_5_0 /E CopyBufferFromTexture /Fh CopyBufferFromTextureCS.Dim2D.inc /Vn CopyBufferFromTextureCS_Dim2D CopyBufferFromTexture.hlsl
fxc /D TEXTURE_DIM=3 /T cs_5_0 /E CopyBufferFromTexture /Fh CopyBufferFromTextureCS.Dim3D.inc /Vn CopyBufferFromTextureCS_Dim3D CopyBufferFromTexture.hlsl
