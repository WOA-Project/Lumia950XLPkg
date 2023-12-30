# Copyright 2018-2019, Bingxing Wang <uefi-oss-projects@imbushuo.net>
# All rights reserved.
#
# This module performs ELF image generation for first-stage bootloaders.
#

Import-Module $PSScriptRoot/redirector.psm1

function Copy-ElfImages {
    param()
    Write-Host "Task: ELF Image generation"

    # Check content (this search pattern is sufficient for our purpose at this moment)
    $BuildContent = Get-ChildItem -Path "Build/Lumia*-AARCH64/**/*.fd" -Recurse
    $LdScript = "ImageResources/FvWrapper.ld"
    if ($BuildContent -eq $false) {
        Write-Error -Message "Build payload is not found."
        return -1
    }

    # Probe GCC
    # Probe GCC. Use the most suitable one
    $ccprefix = Get-GnuAarch64CrossCollectionPath -AllowFallback
    if ($null -eq $ccprefix) { return -1 }
    if ($false -eq (Test-GnuAarch64CrossCollectionVersionRequirements)) {
        Write-Warning "Failed to check GCC version, build may fail!"
    }
    $env:GCC5_AARCH64_PREFIX = $ccprefix

    Write-Output "Use GCC at $($ccprefix) to generate images."

    # Generate image(s).
    foreach ($fd in $BuildContent) {
        # Generate ELF image
        $FdFileSystemPath = $fd.FullName
        $FdDirectory = [System.IO.Path]::GetDirectoryName($FdFileSystemPath)
        $ElfDirectory = [System.IO.Path]::Combine($FdDirectory, "ELF")

        $ElfObjPath = [System.IO.Path]::Combine($ElfDirectory, "FD.o")
        $ElfPath = [System.IO.Path]::Combine($ElfDirectory, "UEFI.elf")

        # Create directory
        if ((Test-Path -Path $ElfDirectory) -eq $false) {
            Write-Host "Create ELF build directory."
            New-Item -ItemType Directory -Path $ElfDirectory -ErrorAction Stop
        }

        # Remove if exists
        if (Test-Path -Path $ElfPath) {
            Remove-Item -Path $ElfPath -Force -ErrorAction Stop
        }

        # Run ELF build
        Write-Host "Build ELF image for $($fd)."
        . "$($ccprefix)objcopy" -I binary -O elf64-littleaarch64 --binary-architecture aarch64 $FdFileSystemPath $ElfObjPath
        . "$($ccprefix)ld" -m aarch64elf $ElfObjPath -T $LdScript -o $ElfPath
        Remove-Item -Path $ElfObjPath -ErrorAction SilentlyContinue
        if ($null -eq $ElfPath) {
            Write-Error "Failed to build ELF image for $($fd)."
            return -1
        }
    }

    Write-Host "Image generation completed. Thank you."
}

# Exports
Export-ModuleMember -Function Copy-ElfImages
