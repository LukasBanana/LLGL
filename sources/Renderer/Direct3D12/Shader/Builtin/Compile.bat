REM Compiles all shaders with fxc
REM Use VisualStudio command line tools to run this batch script

fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips1DCS /Vn g_GenerateMips1DCS /Fh GenerateMips1DCS.cso.inl GenerateMips1D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips1DCS /Vn g_GenerateMips1DCS_OddX /Fh GenerateMips1DCS.OddX.cso.inl GenerateMips1D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips1DCS /Vn g_GenerateMips1DCS_sRGB /Fh GenerateMips1DCS.sRGB.cso.inl GenerateMips1D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips1DCS /Vn g_GenerateMips1DCS_sRGB_OddX /Fh GenerateMips1DCS.sRGB.OddX.cso.inl GenerateMips1D.hlsl

fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips2DCS /Vn g_GenerateMips2DCS /Fh GenerateMips2DCS.cso.inl GenerateMips2D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips2DCS /Vn g_GenerateMips2DCS_OddX /Fh GenerateMips2DCS.OddX.cso.inl GenerateMips2D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=2 /T cs_5_0 /E GenerateMips2DCS /Vn g_GenerateMips2DCS_OddY /Fh GenerateMips2DCS.OddY.cso.inl GenerateMips2D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=3 /T cs_5_0 /E GenerateMips2DCS /Vn g_GenerateMips2DCS_OddXY /Fh GenerateMips2DCS.OddXY.cso.inl GenerateMips2D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Vn g_GenerateMips2DCS_sRGB /Fh GenerateMips2DCS.sRGB.cso.inl GenerateMips2D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Vn g_GenerateMips2DCS_sRGB_OddX /Fh GenerateMips2DCS.sRGB.OddX.cso.inl GenerateMips2D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=2 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Vn g_GenerateMips2DCS_sRGB_OddY /Fh GenerateMips2DCS.sRGB.OddY.cso.inl GenerateMips2D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=3 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips2DCS /Vn g_GenerateMips2DCS_sRGB_OddXY /Fh GenerateMips2DCS.sRGB.OddXY.cso.inl GenerateMips2D.hlsl

fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS /Fh GenerateMips3DCS.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_OddX /Fh GenerateMips3DCS.OddX.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=2 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_OddY /Fh GenerateMips3DCS.OddY.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=3 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_OddXY /Fh GenerateMips3DCS.OddXY.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=4 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_OddZ /Fh GenerateMips3DCS.OddZ.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=5 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_OddXZ /Fh GenerateMips3DCS.OddXZ.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=6 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_OddYZ /Fh GenerateMips3DCS.OddYZ.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=7 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_OddXYZ /Fh GenerateMips3DCS.OddXYZ.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_sRGB /Fh GenerateMips3DCS.sRGB.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_sRGB_OddX /Fh GenerateMips3DCS.sRGB.OddX.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=2 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_sRGB_OddY /Fh GenerateMips3DCS.sRGB.OddY.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=3 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_sRGB_OddXY /Fh GenerateMips3DCS.sRGB.OddXY.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=4 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_sRGB_OddZ /Fh GenerateMips3DCS.sRGB.OddZ.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=5 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_sRGB_OddXZ /Fh GenerateMips3DCS.sRGB.OddXZ.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=6 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_sRGB_OddYZ /Fh GenerateMips3DCS.sRGB.OddYZ.cso.inl GenerateMips3D.hlsl
fxc /Qstrip_reflect /D NPOT_TEXTURE_CLASS=7 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMips3DCS /Vn g_GenerateMips3DCS_sRGB_OddXYZ /Fh GenerateMips3DCS.sRGB.OddXYZ.cso.inl GenerateMips3D.hlsl

dxc -Qstrip_reflect -T cs_6_0 -E StreamOutputDrawArgsCS -Vn g_StreamOutputDrawArgsCS -Fh StreamOutputDrawArgs.dxil.inl StreamOutputDrawArgs.hlsl
