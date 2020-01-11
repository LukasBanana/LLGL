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

fxc /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.OddX.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.OddY.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.OddXY.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=4 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.OddZ.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=5 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.OddXZ.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=6 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.OddYZ.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=7 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.OddXYZ.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.sRGB.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.sRGB.OddX.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.sRGB.OddY.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.sRGB.OddXY.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=4 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.sRGB.OddZ.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=5 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.sRGB.OddXZ.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=6 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.sRGB.OddYZ.cso GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=7 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Fo GenerateMips3DCS.sRGB.OddXYZ.cso GenerateMips3D.hlsl
