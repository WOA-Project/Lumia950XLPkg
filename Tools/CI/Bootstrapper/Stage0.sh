#!/bin/bash
# Travis CI environment bootstrapper (run this as root user)
# Stage 0
#
# Copyright 2018, Bingxing Wang. <i@imbushuo.net>
# All rights reserved.
#

# Permission check
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root." 
   exit 1
fi

echo "Installing build components..."

# Host utilities
apt-get install git-core git
apt-get install build-essential

# PowerShell
# Import the public repository GPG keys
curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -

# Register the Microsoft Ubuntu repository
curl https://packages.microsoft.com/config/ubuntu/14.04/prod.list | tee /etc/apt/sources.list.d/microsoft.list

# Update the list of products
apt-get update

# Install PowerShell
apt-get install -y powershell

# Linaro Toolchains
cd /opt
wget http://releases.linaro.org/components/toolchain/binaries/7.2-2017.11/aarch64-elf/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf.tar.xz
tar xf gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf.tar.xz

# Skip ACPI toolchain (prebuilt tables)

echo "Build components are ready. If you would like to build ACPI tables, please install recent acpica tools."
