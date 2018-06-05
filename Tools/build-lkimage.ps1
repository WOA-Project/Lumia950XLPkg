#!/usr/bin/pwsh
# Copyright 2018, Bingxing Wang <i@imbushuo.net>
# All rights reserved.
#
# This script performs ELF image generation (the last step).
#

Write-Host "Task: ELF Image generation"

# Check content (this search pattern is sufficient for our purpose at this moment)
$BuildContent = Get-ChildItem -Path "Build/Lumia*-AARCH64/**/*.fd" -Recurse

if ($BuildContent -eq $false)
{
    Write-Error -Message "Build payload is not found."
    return -1
}

# Set environment variable for further use.
Write-Output "[PRE] Set environment."
$env:PATH="/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin:/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi/bin:$($env:PATH)"

foreach ($fd in $BuildContent)
{
	# Generate Qualcomm image
	$FdFileSystemPath = $fd.FullName
	$FdDirectory = [System.IO.Path]::GetDirectoryName($FdFileSystemPath)
	$ElfDirectory = [System.IO.Path]::Combine($FdDirectory, "ELF")

	$ElfObjPath = [System.IO.Path]::Combine($ElfDirectory, "FD.o")
	$ElfPath = [System.IO.Path]::Combine($ElfDirectory, "UEFI.elf")

	# Create directory
	if ((Test-Path -Path $ElfDirectory) -eq $false)
	{
		Write-Host "[ELFBuild] Create ELF build directory."
		New-Item -ItemType Directory -Path $ElfDirectory -ErrorAction Stop
	}

	# Remove if exists
	if (Test-Path -Path $ElfPath)
	{
		Remove-Item -Path $ElfPath -Force -ErrorAction Stop
	}

	# Run ELF build
	Write-Host "[ELFBuild] Build ELF image for $($fd)."
	aarch64-elf-objcopy -I binary -O elf64-littleaarch64 --binary-architecture aarch64 $FdFileSystemPath $ElfObjPath
	aarch64-elf-ld -m aarch64elf $ElfObjPath -T Lumia950XLPkg/FvWrapper.ld -o $ElfPath
	Remove-Item -Path $ElfObjPath -ErrorAction SilentlyContinue
	if ($ElfPath -eq $null)
	{
		Write-Error "[ELFBuild] Failed to build ELF image for $($fd)."
		return -1
	}
}
