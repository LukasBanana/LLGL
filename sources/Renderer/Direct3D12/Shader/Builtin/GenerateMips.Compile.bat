REM Compiles all shaders with fxc
REM Use VisualStudio command line tools to run this batch script
fxc /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips1DCS /Fo GenerateMips1DCS.cso GenerateMips1D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips1DCS /Fo GenerateMips1DCS.OddX.cso GenerateMips1D.hlsl
fxc /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips1DCS /Fo GenerateMips1DCS.sRGB.cso GenerateMips1D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips1DCS /Fo GenerateMips1DCS.sRGB.OddX.cso GenerateMips1D.hlsl

fxc /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips2DCS /Fo GenerateMips2DCS.cso GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips2DCS /Fo GenerateMips2DCS.OddX.cso GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /T cs_5_0 /E GenerateMips2DCS /Fo GenerateMips2DCS.OddY.cso GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /T cs_5_0 /E GenerateMips2DCS /Fo GenerateMips2DCS.OddXY.cso GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Fo GenerateMips2DCS.sRGB.cso GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Fo GenerateMips2DCS.sRGB.OddX.cso GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Fo GenerateMips2DCS.sRGB.OddY.cso GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Fo GenerateMips2DCS.sRGB.OddXY.cso GenerateMips2D.hlsl
