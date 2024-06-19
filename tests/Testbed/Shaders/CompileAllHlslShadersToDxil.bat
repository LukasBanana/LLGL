@echo off

dxc -T vs_6_6 -E VSMain -Fo Bindless.VSMain.vs_6_6.dxil Bindless.hlsl
dxc -T ps_6_6 -E PSMain -Fo Bindless.PSMain.ps_6_6.dxil Bindless.hlsl

echo DONE
pause
