#!/bin/bash
# Travis CI environment bootstrapper
# Stage 1
#
# Copyright 2018, Bingxing Wang. <uefi-oss-projects@imbushuo.net>
# All rights reserved.
#

# Checkout EDK2 and switch to UDK2018
echo "Checking out EDK2 workspace"

cd ..
git clone https://github.com/tianocore/edk2

cd edk2
git checkout UDK2018
git pull

cd ..

# Set a link to EDK2 workspace
ln -s $(pwd)/Lumia950XLPkg $(pwd)/edk2/Lumia950XLPkg
ln -s $(pwd)/Lumia950XLPkg/Tools/rundbbuild.sh $(pwd)/edk2/rundbbuild.sh
chmod +x edk2/rundbbuild.sh

# Build EDK2 Tools
cd edk2
make -C BaseTools
