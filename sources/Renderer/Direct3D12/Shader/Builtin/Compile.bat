REM Compiles all shaders with fxc
REM Use VisualStudio command line tools to run this batch script

fxc /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips1DCS /Vn GenerateMips1DCS /Fh GenerateMips1DCS.inc GenerateMips1D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips1DCS /Vn GenerateMips1DCS_OddX /Fh GenerateMips1DCS.OddX.inc GenerateMips1D.hlsl
fxc /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips1DCS /Vn GenerateMips1DCS_sRGB /Fh GenerateMips1DCS.sRGB.inc GenerateMips1D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips1DCS /Vn GenerateMips1DCS_sRGB_OddX /Fh GenerateMips1DCS.sRGB.OddX.inc GenerateMips1D.hlsl

fxc /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips2DCS /Vn GenerateMips2DCS /Fh GenerateMips2DCS.inc GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips2DCS /Vn GenerateMips2DCS_OddX /Fh GenerateMips2DCS.OddX.inc GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /T cs_5_0 /E GenerateMips2DCS /Vn GenerateMips2DCS_OddY /Fh GenerateMips2DCS.OddY.inc GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /T cs_5_0 /E GenerateMips2DCS /Vn GenerateMips2DCS_OddXY /Fh GenerateMips2DCS.OddXY.inc GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Vn GenerateMips2DCS_sRGB /Fh GenerateMips2DCS.sRGB.inc GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Vn GenerateMips2DCS_sRGB_OddX /Fh GenerateMips2DCS.sRGB.OddX.inc GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Vn GenerateMips2DCS_sRGB_OddY /Fh GenerateMips2DCS.sRGB.OddY.inc GenerateMips2D.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Vn GenerateMips2DCS_sRGB_OddXY /Fh GenerateMips2DCS.sRGB.OddXY.inc GenerateMips2D.hlsl

fxc /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS /Fh GenerateMips3DCS.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_OddX /Fh GenerateMips3DCS.OddX.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_OddY /Fh GenerateMips3DCS.OddY.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_OddXY /Fh GenerateMips3DCS.OddXY.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=4 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_OddZ /Fh GenerateMips3DCS.OddZ.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=5 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_OddXZ /Fh GenerateMips3DCS.OddXZ.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=6 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_OddYZ /Fh GenerateMips3DCS.OddYZ.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=7 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_OddXYZ /Fh GenerateMips3DCS.OddXYZ.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_sRGB /Fh GenerateMips3DCS.sRGB.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_sRGB_OddX /Fh GenerateMips3DCS.sRGB.OddX.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_sRGB_OddY /Fh GenerateMips3DCS.sRGB.OddY.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_sRGB_OddXY /Fh GenerateMips3DCS.sRGB.OddXY.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=4 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_sRGB_OddZ /Fh GenerateMips3DCS.sRGB.OddZ.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=5 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_sRGB_OddXZ /Fh GenerateMips3DCS.sRGB.OddXZ.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=6 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_sRGB_OddYZ /Fh GenerateMips3DCS.sRGB.OddYZ.inc GenerateMips3D.hlsl
fxc /D NPOT_TEXTURE_CLASS=7 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn GenerateMips3DCS_sRGB_OddXYZ /Fh GenerateMips3DCS.sRGB.OddXYZ.inc GenerateMips3D.hlsl
