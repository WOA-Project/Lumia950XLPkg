#!/usr/bin/env pwsh
# Copyright 2018-2019, Bingxing Wang <uefi-oss-projects@imbushuo.net>
# All rights reserved.
#
# This script builds EDK2 content.
# EDK2 setup script should be called before invoking this script.
#

Param
(
    [switch] $Clean,
    [switch] $Release
)

Import-Module $PSScriptRoot/PsModules/redirector.psm1
Import-Module $PSScriptRoot/PsModules/elf.psm1
Write-Host "Task: EDK2 build"

# Targets. Ensure corresponding DSC/FDF files exist
# Build all targets on VSTS (phasing out Travis right now) or if user asks to do so
if ($null -ne $env:BUILDALL) {
    Write-Output "User requested build all available targets."
    $availableTargets = @(
        "Lumia950",
        "Lumia950XL"
    )
}
else {
    $availableTargets = @(
        "Lumia950XL"
    )
}

# Check package path.
if ($null -eq (Test-Path -Path "Lumia950XLPkg")) {
    Write-Error "Lumia950XLPkg is not found."
    return -1
}

# Set environment again for legacy compatibility. On newer systems, GCC should be used from package source.
Write-Output "Set legacy environment."
$env:PATH = "/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-elf/bin:/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-eabi/bin:$($env:PATH)"

# Probe GCC
# Probe GCC. Use the most suitable one
$ccprefix = Get-GnuAarch64CrossCollectionPath -AllowFallback
if ($null -eq $ccprefix) { return -1 }
if ($false -eq (Test-GnuAarch64CrossCollectionVersionRequirements)) {
    Write-Warning "Failed to check GCC version, build may fail!"
}
$env:GCC5_AARCH64_PREFIX = $ccprefix

Write-Output "Use GCC at $($ccprefix) to run builds."

# Probe iASL
# $iaslpath = Get-AcpiToolsPath
# if ($null -eq $iaslpath) { return -1 }
# Write-Output "Use iASL at $($iaslpath) to compile SSDTs."

# Build base tools if not exist (dev).
if (((Test-Path -Path "BaseTools") -eq $false) -or ($Clean -eq $true)) {
    Write-Output "Build base tools."
    make -C BaseTools
    if (-not $?) {
        Write-Error "Base tools target failed."
        return $?
    }
}

if ($Clean -eq $true) {
    foreach ($target in $availableTargets) {
        Write-Output "Clean target $($target)."

        if ($Release) {
            build -a AARCH64 -p Lumia950XLPkg/$($target).dsc -t GCC5 clean -b RELEASE
        } else {
            build -a AARCH64 -p Lumia950XLPkg/$($target).dsc -t GCC5 clean
        }

        if (-not $?) {
            Write-Error "Clean target $($target) failed."
            return $?
        }
    }

    # Apply workaround for "NUL"
    Get-ChildItem -Path Build/**/NUL -Recurse | Remove-Item -Force
}

# Check current commit ID and write it into file for SMBIOS reference. (Trim it)
# Check current date and write it into file for SMBIOS reference too. (MM/dd/yyyy)
Write-Output "Stamp build."
# This one is EDK2 base commit
$edk2Commit = git rev-parse HEAD
# This is Lumia950XLPkg package commit
Set-Location Lumia950XLPkg
$commit = git rev-parse HEAD
Set-Location ..
$date = (Get-Date).Date.ToString("MM/dd/yyyy")
$user = (whoami)
try {
    $machine = [System.Net.Dns]::GetHostByName((hostname)).HostName
} catch {
    $machine = "$(hostname)"
}
$owner = "$($user)@$($machine)"
if ($null -eq $machine) { $owner = $user }

if ($commit) {
    $commit = $commit.Substring(0, 8)
    $edk2Commit = $edk2Commit.Substring(0, 8)

    $releaseInfoContent = @(
        "#ifndef __SMBIOS_RELEASE_INFO_H__",
        "#define __SMBIOS_RELEASE_INFO_H__",
        "#ifdef __IMPL_COMMIT_ID__",
        "#undef __IMPL_COMMIT_ID__",
        "#endif",
        "#define __IMPL_COMMIT_ID__ `"$($commit)`"",
        "#ifdef __RELEASE_DATE__",
        "#undef __RELEASE_DATE__",
        "#endif",
        "#define __RELEASE_DATE__ `"$($date)`"",
        "#ifdef __BUILD_OWNER__",
        "#undef __BUILD_OWNER__",
        "#endif",
        "#define __BUILD_OWNER__ `"$($owner)`"",
        "#ifdef __EDK2_RELEASE__",
        "#undef __EDK2_RELEASE__",
        "#endif",
        "#define __EDK2_RELEASE__ `"$($edk2Commit)`"",
        "#endif"
    )

    Set-Content -Path Lumia950XLPkg/Include/Resources/ReleaseInfo.h -Value $releaseInfoContent -ErrorAction SilentlyContinue -Force
}

# Build SSDT tables
# Because this is quick enough, we build for any possible platforms
# $ssdts = Get-ChildItem Lumia950XLPkg/AcpiTables/**/src/SSDT*.asl
# if ($null -ne $ssdts) {
#     foreach ($ssdt in $ssdts) {
#         Write-Output "Build $($ssdt)."
#         $srcDir = [System.IO.Path]::GetDirectoryName($ssdt)
#         $fileName = [System.IO.Path]::GetFileNameWithoutExtension($ssdt)
#         $outDir = "$($srcDir)/../generated"
#         if (!(Test-Path -Path $outDir)) {
#             New-Item -ItemType Directory -Force -Path $outDir
#         }
# 
#         . $iaslpath -p "$($outDir)/$($fileName).aml" $ssdt
#         if (-not $?) {
#             Write-Error "Build SSDT $($ssdt) failed."
#             return $?
#         }
#     }
# }

foreach ($target in $availableTargets) {
    Write-Output "Build Lumia950XLPkg for $($target) (Release = $($Release))."
    if ($Release) {
        build -a AARCH64 -p Lumia950XLPkg/$($target).dsc -t GCC5 -b RELEASE
    } else {
        build -a AARCH64 -p Lumia950XLPkg/$($target).dsc -t GCC5
    }

    if (-not $?) {
        Write-Error "Build target $($target) failed."
        return $?
    }
}

# Invoke ELF build.
Copy-ElfImages
