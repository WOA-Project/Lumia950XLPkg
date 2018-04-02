#!/usr/bin/pwsh
# This script builds EDK2 content.
# EDK2 setup script should be called before invoking this script.

Param
(
    [switch] $Clean,
    [switch] $UseNewerGcc
)

Write-Host "Task: EDK2 build"

# Check path.
if ((Test-Path -Path "Lumia950XLPkg") -eq $null)
{
    Write-Error "Lumia950XLPkg is not found."
    return -1
}

# Set environment again.
Write-Output "[EDK2Build] Set environment."
if ($UseNewerGcc)
{
    $env:PATH="/opt/db-boot-tools:/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin:/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi/bin:/opt/signlk:/opt/skales:$($env:PATH)"
    $env:GCC5_AARCH64_PREFIX="/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin/aarch64-elf-"
}
else
{
    $env:PATH="/opt/db-boot-tools:/opt/gcc-linaro-6.4.1-2017.11-x86_64_aarch64-elf/bin:/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi/bin:/opt/signlk:/opt/skales:$($env:PATH)"
    $env:GCC5_AARCH64_PREFIX="/opt/gcc-linaro-6.4.1-2017.11-x86_64_aarch64-elf/bin/aarch64-elf-"
}

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
    Write-Output "[EDK2Build] Clean Lumia950XLPkg."
    build -a AARCH64 -p Lumia950XLPkg/Lumia950XL.dsc -t GCC5 clean

    if (-not $?)
    {
        Write-Error "[EDK2Build] Clean target failed."
        return $?
    }

    # Apply workaround for "NUL"
    Get-ChildItem -Path Build/**/NUL -Recurse | Remove-Item -Force
}

Write-Output "[EDK2Build] Build Lumia950XLPkg (DEBUG)."
build -a AARCH64 -p Lumia950XLPkg/Lumia950XL.dsc -t GCC5
if (-not $?)
{
    Write-Error "[EDK2Build] Build target failed."
    return $?
}
