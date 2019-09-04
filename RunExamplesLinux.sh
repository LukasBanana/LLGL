#!/bin/bash
BUILD_DIR="build/build"

if [ "$#" -eq 1 ]; then
    BUILD_DIR=$1
elif [ -f "build/build/Example_HelloTriangle" ]; then
    BUILD_DIR="build/build"
elif [ -f "bin/x64/Linux/Example_HelloTriangle" ]; then
    BUILD_DIR="bin/x64/Linux"
else
    echo "error: build folder not found: $BUILD_DIR"
    exit 1
fi

PS3='select example: '
options=(
	"Hello Triangle"
	"Tessellation"
	"Texturing"
	"Queries"
	"Render Target"
	"Multi Context"
	"Buffer Array"
	"Compute Shader"
	"Stream Output"
	"Instancing"
	"Post Processing"
	"Shadow Mapping"
	"Animation"
	"Stencil Buffer"
	"Volume Rendering"
)
select opt in "${options[@]}"
do
	case $opt in
	"${options[0]}")
		(cd examples/Cpp/HelloTriangle; ../../../$BUILD_DIR/Example_HelloTriangle)
		;;
	"${options[1]}")
		(cd examples/Cpp/Tessellation; ../../../$BUILD_DIR/Example_Tessellation)
		;;
	"${options[2]}")
		(cd examples/Cpp/Texturing; ../../../$BUILD_DIR/Example_Texturing)
		;;
	"${options[3]}")
		(cd examples/Cpp/Queries; ../../../$BUILD_DIR/Example_Queries)
		;;
	"${options[4]}")
		(cd examples/Cpp/RenderTarget; ../../../$BUILD_DIR/Example_RenderTarget)
		;;
	"${options[5]}")
		(cd examples/Cpp/MultiContext; ../../../$BUILD_DIR/Example_MultiContext)
		;;
	"${options[6]}")
		(cd examples/Cpp/BufferArray; ../../../$BUILD_DIR/Example_BufferArray)
		;;
	"${options[7]}")
		(cd examples/Cpp/ComputeShader; ../../../$BUILD_DIR/Example_ComputeShader)
		;;
	"${options[8]}")
		(cd examples/Cpp/StreamOutput; ../../../$BUILD_DIR/Example_StreamOutput)
		;;
	"${options[9]}")
		(cd examples/Cpp/Instancing; ../../../$BUILD_DIR/Example_Instancing)
		;;
	"${options[10]}")
		(cd examples/Cpp/PostProcessing; ../../../$BUILD_DIR/Example_PostProcessing)
		;;
	"${options[11]}")
		(cd examples/Cpp/ShadowMapping; ../../../$BUILD_DIR/Example_ShadowMapping)
		;;
	"${options[12]}")
		(cd examples/Cpp/Animation; ../../../$BUILD_DIR/Example_Animation)
		;;
	"${options[13]}")
		(cd examples/Cpp/StencilBuffer; ../../../$BUILD_DIR/Example_StencilBuffer)
		;;
	"${options[14]}")
		(cd examples/Cpp/VolumeRendering; ../../../$BUILD_DIR/Example_VolumeRendering)
		;;
	*)
		echo "invalid selection";;
	esac
done
