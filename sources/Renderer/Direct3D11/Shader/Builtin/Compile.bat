REM Compiles all shaders with fxc
REM Use VisualStudio command line tools to run this batch script

fxc /D TEXTURE_DIM=1 /T cs_5_0 /E CopyTextureFromBuffer /Fo CopyTextureFromBufferCS.Dim1D.cso CopyTextureFromBuffer.hlsl
fxc /D TEXTURE_DIM=2 /T cs_5_0 /E CopyTextureFromBuffer /Fo CopyTextureFromBufferCS.Dim2D.cso CopyTextureFromBuffer.hlsl
fxc /D TEXTURE_DIM=3 /T cs_5_0 /E CopyTextureFromBuffer /Fo CopyTextureFromBufferCS.Dim3D.cso CopyTextureFromBuffer.hlsl

fxc /D TEXTURE_DIM=1 /T cs_5_0 /E CopyBufferFromTexture /Fo CopyBufferFromTextureCS.Dim1D.cso CopyBufferFromTexture.hlsl
fxc /D TEXTURE_DIM=2 /T cs_5_0 /E CopyBufferFromTexture /Fo CopyBufferFromTextureCS.Dim2D.cso CopyBufferFromTexture.hlsl
fxc /D TEXTURE_DIM=3 /T cs_5_0 /E CopyBufferFromTexture /Fo CopyBufferFromTextureCS.Dim3D.cso CopyBufferFromTexture.hlsl
