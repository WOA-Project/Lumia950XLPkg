#!/bin/bash
# Travis CI builder
#
# Copyright 2018, Bingxing Wang. <i@imbushuo.net>
# All rights reserved.
#

# Export AArch64 Tools prefix
export GCC5_AARCH64_PREFIX=/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin/aarch64-elf-

# Go to EDK2 workspace
cd ..
cd edk2

# Start build
echo "Start build..."
. rundbbuild.sh --950xl --development

# Check if we have both FD ready
if [ ! -f Build/Lumia950-AARCH64/DEBUG_GCC5/FV/MSM8992_EFI.fd ]; then
    echo "Dude, where's your MSM8992 FD?"
    exit 1
fi

if [ ! -f Build/Lumia950XL-AARCH64/DEBUG_GCC5/FV/MSM8994_EFI.fd ]; then
    echo "Dude, where's your MSM8994 FD?"
    exit 1
fi
