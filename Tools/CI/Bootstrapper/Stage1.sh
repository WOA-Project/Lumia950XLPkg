#!/bin/bash
# Generic CI environment bootstrapper
# Stage 1
#
# Copyright 2018-2019, Bingxing Wang. <uefi-oss-projects@imbushuo.net>
# All rights reserved.
#

# Checkout EDK2 at master and recent commit only
echo "Checking out EDK2 workspace"

cd ..
#git clone --single-branch --depth 1 --recurse-submodules --branch master https://github.com/tianocore/edk2
# Checkout 838c730fe61578847823da1b815e4c9c5ee1bed4 (LKG)
# We do not use the revision param for compatibility with older git versions
mkdir edk2
cd edk2
git init
git remote add origin https://github.com/tianocore/edk2
git fetch --depth=1 origin 838c730fe61578847823da1b815e4c9c5ee1bed4
git checkout FETCH_HEAD
git submodule init
git submodule update
cd ..

# Set a link to EDK2 workspace
ln -s $(pwd)/Lumia950XLPkg $(pwd)/edk2/Lumia950XLPkg
ln -s $(pwd)/Lumia950XLPkg/Tools/rundbbuild.sh $(pwd)/edk2/rundbbuild.sh
chmod +x edk2/rundbbuild.sh

# Build EDK2 Tools
cd edk2
make -C BaseTools
