#!/bin/bash
# Generic CI environment bootstrapper
# Stage 1
#
# Copyright 2018-2019, Bingxing Wang. <uefi-oss-projects@imbushuo.net>
# All rights reserved.
#

# Checkout EDK2 at master and recent commit only
echo "Checking out EDK2 workspace"

mkdir edk2
cd edk2

# create and initialize an empty repository
git init
 
# add a remote named origin for the repository at https://github.com/tianocore/edk2.git
git remote add origin https://github.com/tianocore/edk2.git
 
# fetch a commit using its hash
git fetch --depth 1 --recurse-submodules origin 03e77558d4939b9c21e94f03072360e9b00bb559
 
# reset repository to that commit
git reset --hard FETCH_HEAD

# fetch submodules
git submodule foreach --recursive git fetch

cd ..

# Set a link to EDK2 workspace
ln -s $(pwd)/Lumia950XLPkg $(pwd)/edk2/Lumia950XLPkg
ln -s $(pwd)/Lumia950XLPkg/Tools/rundbbuild.sh $(pwd)/edk2/rundbbuild.sh
chmod +x edk2/rundbbuild.sh

# Build EDK2 Tools
cd edk2
make -C BaseTools
