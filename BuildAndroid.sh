#!/bin/bash

IFS="" # Include whitespaces when expanding arrays of strings
SOURCE_DIR=$PWD
OUTPUT_DIR="build_android"
SKIP_VALIDATION=0
CLEAR_CACHE=0
ENABLE_VULKAN="OFF"
ENABLE_EXAMPLES="ON"
BUILD_TYPE="Release"
PROJECT_ONLY=0
STATIC_LIB="OFF"
ANDROID_CXX_LIB="c++_shared" # c++_shared/c++_static
VERBOSE=0
GENERATOR="CodeBlocks - Unix Makefiles"
ANDROID_ABI=x86_64
ANDROID_API_LEVEL=21
SUPPORTED_ANDROID_ABIS=("arm64-v8a" "armeabi-v7a" "x86" "x86_64")
GLES_VER="OpenGLES 3.0"
BUILD_APPS=0

print_help()
{
    echo "USAGE:"
    echo "  BuildAndroid.sh OPTIONS* [OUTPUT_DIR]"
    echo "OPTIONS:"
    echo "  -c, --clear-cache ......... Clear CMake cache and rebuild"
    echo "  -d, --debug ............... Configure Debug build (default is Release)"
    echo "  -h, --help ................ Print this help documentation and exit"
    echo "  -p, --project-only [=G] ... Build project with CMake generator (default is CodeBlocks)"
    echo "  -s, --static-lib .......... Build static lib (default is shared lib)"
    echo "  -v, --verbose ............. Print additional information"
    echo "  --abi=ABI ................. Set Android ABI (default is x86_64; accepts 'all')"
    echo "  --api-level=VERSION ....... Set Android API level (default is 21)"
    echo "  --apps .................... Generate Android Studio projects to build example apps (implies '--abi=all -s')"
    echo "  --gles=VER ................ Enables the maximum OpenGLES version: 300 (default), 310, or 320"
    echo "  --vulkan .................. Include Vulkan renderer"
    echo "  --no-examples ............. Exclude example projects"
    echo "NOTES:"
    echo "  Default output directory is '$OUTPUT_DIR'"
}

# Parse arguments
for ARG in "$@"; do
    if [ "$ARG" = "-h" ] || [ "$ARG" = "--help" ]; then
        print_help
        exit 0
    elif [ "$ARG" = "-c" ] || [ "$ARG" = "--clear-cache" ]; then
        CLEAR_CACHE=1
    elif [ "$ARG" = "-d" ] || [ "$ARG" = "--debug" ]; then
        BUILD_TYPE="Debug"
    elif [ "$ARG" = "-p" ] || [ "$ARG" = "--project-only" ]; then
        PROJECT_ONLY=1
    elif [[ "$ARG" == -p=* ]]; then
        PROJECT_ONLY=1
        GENERATOR="${ARG:3}"
    elif [[ "$ARG" == --project-only=* ]]; then
        PROJECT_ONLY=1
        GENERATOR="${ARG:15}"
    elif [ "$ARG" = "-s" ] || [ "$ARG" = "--static-lib" ]; then
        STATIC_LIB="ON"
        ANDROID_CXX_LIB="c++_static"
    elif [ "$ARG" = "-v" ] || [ "$ARG" = "--verbose" ]; then
        VERBOSE=1
    elif [[ "$ARG" == --abi=* ]]; then
        ANDROID_ABI="${ARG:6}"
    elif [[ "$ARG" == --api-level=* ]]; then
        ANDROID_API_LEVEL=${ARG:12}
    elif [[ "$ARG" == --gles=* ]]; then
        GLES_VER_NO=${ARG:7}
        case $GLES_VER_NO in
            320) GLES_VER="OpenGLES 3.2" ;;
            310) GLES_VER="OpenGLES 3.1" ;;
            300) GLES_VER="OpenGLES 3.0" ;;
            *) echo "Unknown GLES version: $GLES_VER_NO; Must be 320, 310, or 300"; exit 1 ;;
        esac
    elif [ "$ARG" = "--vulkan" ]; then
        ENABLE_VULKAN="ON"
    elif [ "$ARG" = "--no-examples" ]; then
        ENABLE_EXAMPLES="OFF"
    elif [ "$ARG" = "--apps" ]; then
        BUILD_APPS=1
        ANDROID_ABI="all"
        STATIC_LIB="ON"
        ANDROID_CXX_LIB="c++_static"
    else
        OUTPUT_DIR="$ARG"
    fi
done

# Make sure API level is high enough when Vulkan is enabled
if [ "$ENABLE_VULKAN" = "ON" ]; then
    if [ $ANDROID_API_LEVEL -lt 28 ]; then
        if [ $VERBOSE -ne 0 ]; then
            echo "Clamping API level to 28 for Vulkan support (--api-level=$ANDROID_API_LEVEL)"
        fi
        ANDROID_API_LEVEL=28
    fi
fi

# Find Android NDK installation
NDK_ROOT=""
if [ -z "$ANDROID_NDK_HOME" ]; then
    if [ -z "$ANDROID_NDK_ROOT" ]; then
        echo "Error: Neither environment variable 'ANDROID_NDK_HOME' nor 'ANDROID_NDK_ROOT' are set"
        exit 1
    else
        NDK_ROOT="$ANDROID_NDK_ROOT"
    fi
else
    NDK_ROOT="$ANDROID_NDK_HOME"
fi

ANDROID_CMAKE_TOOLCHAIN="${NDK_ROOT}/build/cmake/android.toolchain.cmake"

if [ ! -f "$ANDROID_CMAKE_TOOLCHAIN" ]; then
    echo "Error: CMake toolchain not found: ${ANDROID_CMAKE_TOOLCHAIN}"
    exit 1
fi

# Ensure we are inside the repository folder
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: File not found: CMakeLists.txt"
    exit 1
fi

# Make output build folder
if [ $CLEAR_CACHE = 1 ] && [ -d "$OUTPUT_DIR" ]; then
    rm -rf "$OUTPUT_DIR"
fi

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir "$OUTPUT_DIR"
fi

# Checkout external depenencies
GAUSSIAN_LIB_DIR="GaussianLib/include"

if [ -f "$SOURCE_DIR/external/$GAUSSIAN_LIB_DIR/Gauss/Gauss.h" ]; then
    GAUSSIAN_LIB_DIR=$(realpath "$SOURCE_DIR/external/$GAUSSIAN_LIB_DIR")
else
    if [ ! -d "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR" ]; then
        (cd "$OUTPUT_DIR" && git clone https://github.com/LukasBanana/GaussianLib.git)
    fi
    GAUSSIAN_LIB_DIR=$(realpath "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR")
fi

# Print additional information if in verbose mode
if [ $VERBOSE -ne 0 ]; then
    echo "GAUSSIAN_LIB_DIR=$GAUSSIAN_LIB_DIR"
    if [ $PROJECT_ONLY -eq 0 ]; then
        echo "BUILD_TYPE=$BUILD_TYPE"
    else
        echo "GENERATOR=$GENERATOR"
    fi
    echo "ANDROID_ABI=$ANDROID_ABI"
    echo "ANDROID_API_LEVEL=$ANDROID_API_LEVEL"
fi

# Build into output directory (this syntax requires CMake 3.13+)
BASE_OPTIONS=(
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_CMAKE_TOOLCHAIN"
    -DANDROID_PLATFORM=$ANDROID_API_LEVEL
    -DANDROID_STL=$ANDROID_CXX_LIB
    -DANDROID_CPP_FEATURES="rtti exceptions"
    -DLLGL_BUILD_WRAPPER_C99=ON
    -DLLGL_BUILD_RENDERER_OPENGLES3=ON
    -DLLGL_GL_ENABLE_OPENGLES=$GLES_VER
    -DLLGL_BUILD_RENDERER_NULL=$ENABLE_NULL
    -DLLGL_BUILD_RENDERER_VULKAN=$ENABLE_VULKAN
    -DLLGL_VK_ENABLE_SPIRV_REFLECT=$ENABLE_VULKAN
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES
    -DLLGL_BUILD_TESTS=OFF
    -DLLGL_BUILD_STATIC_LIB=$STATIC_LIB
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR"
    -S "$SOURCE_DIR"
)

if [ $VERBOSE -ne 0 ]; then
    BASE_OPTIONS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

build_with_android_abi()
{
    CURRENT_ANDROID_ABI=$1
    CURRENT_OUTPUT_DIR=$2

    if [ $VERBOSE -ne 0 ]; then
        echo "Build: ABI=$CURRENT_ANDROID_ABI, Output=$CURRENT_OUTPUT_DIR"
    fi

    OPTIONS=(
        ${BASE_OPTIONS[@]}
        -DANDROID_ABI=$CURRENT_ANDROID_ABI
        -B "$CURRENT_OUTPUT_DIR"
    )

    if [ ! -d "$CURRENT_OUTPUT_DIR" ]; then
        mkdir "$CURRENT_OUTPUT_DIR"
    fi

    if [ $PROJECT_ONLY -eq 0 ]; then
        cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]}
        cmake --build "$CURRENT_OUTPUT_DIR"
    else
        cmake ${OPTIONS[@]} -G "$GENERATOR"
    fi
}

if [ $ANDROID_ABI = "all" ]; then
    declare -i ABI_INDEX=1
    for ABI in ${SUPPORTED_ANDROID_ABIS[@]}; do
        if [ $VERBOSE -ne 0 ]; then
            echo "[${ABI_INDEX}/${#SUPPORTED_ANDROID_ABIS[@]}] Building with Android ABI: $ABI"
            ABI_INDEX+=1
        fi
        build_with_android_abi $ABI "${OUTPUT_DIR}/${ABI}"
    done
else
    build_with_android_abi $ANDROID_ABI "$OUTPUT_DIR"
fi

# Build project solutions for example apps
generate_app_project()
{
    CURRENT_PROJECT=$1

    echo "Generate app project: $CURRENT_PROJECT"

    # Get source folder
    ASSETS_SOURCE_DIR="$SOURCE_DIR/examples/Shared/Assets"

    if [[ "${CURRENT_PROJECT:0:4}" == "C99_" ]]; then
        PROJECT_SOURCE_DIR="$SOURCE_DIR/examples/C99"
        if [[ "$CURRENT_PROJECT" == *D ]]; then
            PROJECT_SOURCE_DIR="$PROJECT_SOURCE_DIR/${CURRENT_PROJECT:4:-1}"
        else
            PROJECT_SOURCE_DIR="$PROJECT_SOURCE_DIR/${CURRENT_PROJECT:4}"
        fi
    else
        PROJECT_SOURCE_DIR="$SOURCE_DIR/examples/Cpp"
        if [[ "$CURRENT_PROJECT" == *D ]]; then
            PROJECT_SOURCE_DIR="$PROJECT_SOURCE_DIR/${CURRENT_PROJECT:0:-1}"
        else
            PROJECT_SOURCE_DIR="$PROJECT_SOURCE_DIR/$CURRENT_PROJECT"
        fi
    fi

    # Get destination folder
    APP_ROOT="${OUTPUT_DIR}/apps/Example_$CURRENT_PROJECT"

    BIN_ROOT=${OUTPUT_DIR}/${ABI}/build

    # Create folder structure
    mkdir -p "$APP_ROOT"
    PLATFORM_SOURCE_DIR="$SOURCE_DIR/examples/Shared/Platform/Android"
    cp -r "$PLATFORM_SOURCE_DIR/app" "$APP_ROOT"
    cp "$PLATFORM_SOURCE_DIR/build.gradle" "$APP_ROOT/build.gradle"
    cp "$PLATFORM_SOURCE_DIR/settings.gradle" "$APP_ROOT/settings.gradle"

    mkdir -p "$APP_ROOT/gradle/wrapper"
    cp -r "$PLATFORM_SOURCE_DIR/gradle/wrapper/gradle-wrapper.properties" "$APP_ROOT/gradle/wrapper/gradle-wrapper.properties"

    # Copy binary files into JNI lib folders for respective ABI
    for ABI in ${SUPPORTED_ANDROID_ABIS[@]}; do
        LIB_FILENAME="libExample_${CURRENT_PROJECT}.so"
        SRC_LIB_PATH="$OUTPUT_DIR/$ABI/build"
        DST_LIB_PATH="$APP_ROOT/app/src/main/jniLibs/$ABI"
        mkdir -p "$DST_LIB_PATH"
        cp "$SRC_LIB_PATH/$LIB_FILENAME" "$DST_LIB_PATH/$LIB_FILENAME"
    done

    # Replace meta data
    sed -i "s/LLGL_PROJECT_NAME/Example_${CURRENT_PROJECT}/g" "$APP_ROOT/app/src/main/AndroidManifest.xml"
    sed -i "s/LLGL_APP_NAME/${CURRENT_PROJECT}/g" "$APP_ROOT/app/src/main/res/values/strings.xml"
    sed -i "s/LLGL_APP_ID/${CURRENT_PROJECT}/g" "$APP_ROOT/app/build.gradle"

    # Find all required assets in Android.assets.txt file of respective project directory and copy them into app folder
    ASSET_DIR="$APP_ROOT/app/src/main/assets"
    mkdir -p "$ASSET_DIR"

    ASSETS_LIST_FILE=$(find "$PROJECT_SOURCE_DIR" -type f -name *.assets.txt)
    if [ -f "$ASSETS_LIST_FILE" ]; then
        # Read *.assets.txt file line-by-line into array and make sure '\r' character is not present (on Win32 platform)
        readarray -t ASSET_FILTERS < <(tr -d '\r' < "$ASSETS_LIST_FILE")
        ASSET_FILES=()
        for FILTER in ${ASSET_FILTERS[@]}; do
            # Search for patterns in both the shared assets and current project folder
            for FILE in $ASSETS_SOURCE_DIR/$FILTER; do
                if [ -f "$FILE" ]; then
                    ASSET_FILES+=( "$FILE" )
                fi
            done
            for FILE in $PROJECT_SOURCE_DIR/$FILTER; do
                if [ -f "$FILE" ]; then
                    ASSET_FILES+=( "$FILE" )
                fi
            done
        done

        # Copy all asset files into destination folder
        for FILE in ${ASSET_FILES[@]}; do
            if [ $VERBOSE -ne 0 ]; then
                echo "Copy asset: $(basename $FILE)"
            fi
            cp "$FILE" "$ASSET_DIR/$(basename $FILE)"
        done
    fi

    # Find all shaders and copy them into app folder
    for FILE in $PROJECT_SOURCE_DIR/*.vert \
                $PROJECT_SOURCE_DIR/*.geom \
                $PROJECT_SOURCE_DIR/*.tesc \
                $PROJECT_SOURCE_DIR/*.tese \
                $PROJECT_SOURCE_DIR/*.frag \
                $PROJECT_SOURCE_DIR/*.comp \
                $PROJECT_SOURCE_DIR/*.spv; do
        if [ -f "$FILE" ]; then
            if [ $VERBOSE -ne 0 ]; then
                echo "Copy shader: $(basename $FILE)"
            fi
            cp "$FILE" "$ASSET_DIR/$(basename $FILE)"
        fi
    done
}

if [ $BUILD_APPS -ne 0 ]; then

    BIN_FILE_BASE="${OUTPUT_DIR}/${SUPPORTED_ANDROID_ABIS[0]}/build/libExample_"
    BIN_FILE_BASE_LEN=${#BIN_FILE_BASE}
    EXAMPLE_BIN_FILES=(${BIN_FILE_BASE}*.so)

    for BIN_FILE in ${EXAMPLE_BIN_FILES[@]}; do
        if { [ $BUILD_TYPE = "Debug" ] && [[ $BIN_FILE == *D.so ]] } || { [ ! $BUILD_TYPE = "Debug" ] && ! [[ $BIN_FILE == *D.so ]] }; then
            BIN_FILE_LEN=${#BIN_FILE}
            PROJECT_NAME=${BIN_FILE:BIN_FILE_BASE_LEN:BIN_FILE_LEN-BIN_FILE_BASE_LEN-3}
            generate_app_project $PROJECT_NAME
        fi
    done

fi
