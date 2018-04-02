#!/usr/bin/pwsh
# This script performs LK image generation (the last step).

Write-Host "Task: LK Image generation"

# Check content
$BuildContent = Get-ChildItem -Path "Build/Lumia950XL-AARCH64/**/*.fd" -Recurse

if ($BuildContent -eq $false)
{
    Write-Error -Message "Build payload is not found."
    return -1
}

# Set environment variable for further use.
Write-Output "[PRE] Set environment."
$env:PATH="/opt/db-boot-tools:/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin:/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi/bin:/opt/signlk:/opt/skales:$($env:PATH)"

if ($BuildContent -eq $null)
{
    Write-Warning -Message "Build content not found."
    return
}

# Generate Qualcomm image
$FdFileSystemPath = $BuildContent[0].FullName
$FdDirectory = [System.IO.Path]::GetDirectoryName($FdFileSystemPath)
$ElfDirectory = [System.IO.Path]::Combine($FdDirectory, "ELF")

$ElfObjPath = [System.IO.Path]::Combine($ElfDirectory, "FD.o")
$ElfPath = [System.IO.Path]::Combine($ElfDirectory, "UEFI.elf")

# Create directory
if ((Test-Path -Path $ElfDirectory) -eq $false)
{
    Write-Host "[LKBuild] Create ELF build directory."
    New-Item -ItemType Directory -Path $ElfDirectory -ErrorAction Stop
}

# Remove if exists
if (Test-Path -Path $ElfPath)
{
    Remove-Item -Path $ElfPath -Force -ErrorAction Stop
}

# Run ELF build
Write-Host "[LKBuild] Build ELF image."
aarch64-elf-objcopy -I binary -O elf64-littleaarch64 --binary-architecture aarch64 $FdFileSystemPath $ElfObjPath
aarch64-elf-ld -m aarch64elf $ElfObjPath -T Lumia950XLPkg/FvWrapper.ld -o $ElfPath
Remove-Item -Path $ElfObjPath -ErrorAction SilentlyContinue
if ($ElfPath -eq $null)
{
    Write-Error "[LKBuild] Failed to build ELF image."
    return -1
}
