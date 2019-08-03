REM Compiles all shaders with fxc
REM Use VisualStudio command line tools to run this batch script
fxc /D NPOT_TEXTURE_CLASS=0 /T cs_5_0 /E GenerateMipsCS /Fo GenerateMipsCS.cso GenerateMips.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /T cs_5_0 /E GenerateMipsCS /Fo GenerateMipsCS.OddX.cso GenerateMips.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /T cs_5_0 /E GenerateMipsCS /Fo GenerateMipsCS.OddY.cso GenerateMips.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /T cs_5_0 /E GenerateMipsCS /Fo GenerateMipsCS.OddXY.cso GenerateMips.hlsl
fxc /D NPOT_TEXTURE_CLASS=0 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMipsCS /Fo GenerateMipsCS.sRGB.cso GenerateMips.hlsl
fxc /D NPOT_TEXTURE_CLASS=1 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMipsCS /Fo GenerateMipsCS.sRGB.OddX.cso GenerateMips.hlsl
fxc /D NPOT_TEXTURE_CLASS=2 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMipsCS /Fo GenerateMipsCS.sRGB.OddY.cso GenerateMips.hlsl
fxc /D NPOT_TEXTURE_CLASS=3 /D LINEAR_TO_SRGB=1 /T cs_5_0 /E GenerateMipsCS /Fo GenerateMipsCS.sRGB.OddXY.cso GenerateMips.hlsl
