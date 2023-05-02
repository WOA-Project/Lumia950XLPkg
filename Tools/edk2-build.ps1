#!/usr/bin/env pwsh
# Copyright 2018-2019, Bingxing Wang <uefi-oss-projects@imbushuo.net>
# All rights reserved.
#
# This script builds EDK2 content.
# EDK2 setup script should be called before invoking this script.
#

Import-Module $PSScriptRoot/PsModules/elf.psm1

# Invoke ELF build.
Copy-ElfImages
