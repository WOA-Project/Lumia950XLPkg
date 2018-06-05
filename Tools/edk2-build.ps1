#!/usr/bin/pwsh
# Copyright 2018, Bingxing Wang <i@imbushuo.net>
# All rights reserved.
#
# This script builds EDK2 content.
# EDK2 setup script should be called before invoking this script.
#

Param
(
    [switch] $Clean,
    [switch] $UseNewerGcc
)

Write-Host "Task: EDK2 build"

# Targets. Ensure corresponding DSC/FDF files exist
$availableTargets = @(
	"Lumia950",
	"Lumia950XL"
)

# Check package path.
if ((Test-Path -Path "Lumia950XLPkg") -eq $null)
{
    Write-Error "Lumia950XLPkg is not found."
    return -1
}

# Set environment again.
Write-Output "[EDK2Build] Set environment."
$env:PATH="/opt/db-boot-tools:/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin:/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi/bin:$($env:PATH)"
$env:GCC5_AARCH64_PREFIX="/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin/aarch64-elf-"

# Build base tools if not exist (dev).
if (((Test-Path -Path "BaseTools") -eq $false) -or ($Clean -eq $true))
{
    Write-Output "[EDK2Build] Build base tools."
    make -C BaseTools
    if (-not $?)
    {
        Write-Error "[EDK2Build] Base tools target failed."
        return $?
    }
}

if ($Clean -eq $true)
{
	foreach ($target in $availableTargets)
	{
		Write-Output "[EDK2Build] Clean target $($target)."
		build -a AARCH64 -p Lumia950XLPkg/$($target).dsc -t GCC5 clean

		if (-not $?)
		{
			Write-Error "[EDK2Build] Clean target $($target) failed."
			return $?
		}
	}

    # Apply workaround for "NUL"
	Get-ChildItem -Path Build/**/NUL -Recurse | Remove-Item -Force
}

# Check current commit ID and write it into file for SMBIOS reference.
Write-Output "[EDK2Build] Stamp build."
$commit = git rev-parse HEAD
if ($commit)
{
	$releaseInfoContent = @(
		"#ifndef __SMBIOS_RELEASE_INFO_H__",
		"#define __SMBIOS_RELEASE_INFO_H__",
		"#ifdef __IMPL_COMMIT_ID__",
		"#undef __IMPL_COMMIT_ID__",
		"#endif",
		"#define __IMPL_COMMIT_ID__ `"$($commit)`"",
		"#endif"
	)

	Set-Content -Path Lumia950XLPkg/Driver/SmBiosTableDxe/ReleaseInfo.h -Value $releaseInfoContent -ErrorAction SilentlyContinue -Force
}

foreach ($target in $availableTargets)
{
	Write-Output "[EDK2Build] Build Lumia950XLPkg for $($target) (DEBUG)."
	build -a AARCH64 -p Lumia950XLPkg/$($target).dsc -t GCC5

	if (-not $?)
	{
		Write-Error "[EDK2Build] Build target $($target) failed."
		return $?
	}
}
