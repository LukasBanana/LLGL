#!/bin/bash

find_package_manager()
{
    declare -A osInfo;
    osInfo[/etc/alpine-release]=apk
    osInfo[/etc/arch-release]=pacman
    osInfo[/etc/debian_version]=apt-get
    osInfo[/etc/gentoo-release]=emerge
    osInfo[/etc/redhat-release]=yum
    osInfo[/etc/SuSE-release]=zypp

    for F in ${!osInfo[@]}; do
        if [[ -f $F ]]; then
            echo ${osInfo[$F]}
            return 0
        fi
    done

    return 1 # Could not identify package manager
}

list_missing_packages_generic()
{
    ARGS=("$@")
    CHECK_CMD="${ARGS[0]}"
    REQUEST=("${ARGS[@]:1}")
    MISSING=""

    for PKG in ${REQUEST[@]}; do
        eval $CHECK_CMD "$PKG" &> /dev/null
        if [ $? -ne 0 ]; then
            if [ ! -z "$MISSING" ]; then
                MISSING="$MISSING $PKG"
            else
                MISSING="$PKG"
            fi
        fi
    done

    if [ ! -z "$MISSING" ]; then
        echo "$MISSING"
    fi
}

list_missing_packages_arch()
{
    list_missing_packages_generic "pacman -Qi" $@
}

list_missing_packages_debian()
{
    list_missing_packages_generic "dpkg -s" $@
}

list_missing_packages_redhat()
{
    list_missing_packages_generic "yum list installed" $@
}

report_missing_packages()
{
    CMD=$1
    MISSING=$2
    if [ ! -z "$MISSING" ]; then
        echo "missing packages! enter the following command to install them:"
        echo "$CMD $MISSING"
        exit 1
    fi
}

KERNEL_NAME=$(uname -s)

if [[ $KERNEL_NAME == MSYS_NT* ]] || [[ $KERNEL_NAME == MINGW32_NT* ]] || [[ $KERNEL_NAME == MINGW64_NT* ]]; then
    # Check packages for MSYS2 on Windows
    MISSING=$( list_missing_packages_arch \
        git mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-glew mingw-w64-x86_64-freeglut )
    report_missing_packages "pacman -S" "$MISSING"
else
    # Check packages for respective Linux distribution
    PKG_MGR=$(find_package_manager)

    if [ "$PKG_MGR" = "pacman" ]; then
        MISSING=$( list_missing_packages_arch \
            git cmake libx11 libxrandr libgl )
        report_missing_packages "sudo pacman -S" "$MISSING"
    elif [ "$PKG_MGR" = "apt-get" ]; then
        MISSING=$( list_missing_packages_debian \
            git cmake libx11-dev libxrandr-dev mesa-common-dev libglu1-mesa-dev freeglut3-dev )
        report_missing_packages "sudo apt-get install" "$MISSING"
    elif [ "$PKG_MGR" = "yum" ]; then
        MISSING=$( list_missing_packages_redhat \
            git cmake libx11-devel libXrandr-devel mesa-libGL-devel )
        report_missing_packages "sudo yum install" "$MISSING"
    else
        echo error: could not identify Linux distribution
        exit 1
    fi
fi

