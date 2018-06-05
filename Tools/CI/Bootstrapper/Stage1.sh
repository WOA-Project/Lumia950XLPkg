#!/bin/bash
# Travis CI environment bootstrapper
# Stage 1
#
# Copyright 2018, Bingxing Wang. <i@imbushuo.net>
# All rights reserved.
#

# Checkout EDK2 and switch to UDK2018
echo "Checking out EDK2 workspace"

cd ..
git clone https://github.com/tianocore/edk2

cd edk2
git checkout 0e2a5749d89c96e3e17ea458365d2e5296c807e2
git pull

cd ..

# Set a link to EDK2 workspace
ln -s $(pwd)/Lumia950XLPkg $(pwd)/edk2/Lumia950XLPkg
ln -s $(pwd)/Lumia950XLPkg/Tools/rundbbuild.sh $(pwd)/edk2/rundbbuild.sh
chmod +x edk2/rundbbuild.sh

# Build EDK2 Tools
cd edk2
make -C BaseTools
