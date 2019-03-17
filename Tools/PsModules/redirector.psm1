# Copyright 2018-2019, Bingxing Wang <uefi-oss-projects@imbushuo.net>
# All rights reserved.
#
# This script module provides shared utilities.
#

function Get-GnuAarch64CrossCollectionPath 
{
    param(
        [switch]$AllowFallback
    )

    $possibleGccCommands = @(
        "aarch64-linux-gnu-gcc",
        "aarch64-elf-gcc",
        "aarch64-none-elf-gcc"
    )

    $ccprefix = $null
    foreach ($gccCommand in $possibleGccCommands)
    {
        $path = which $gccCommand
        if ($null -ne $path)
        {
            # Trim "gcc"
            $ccprefix = $path.Substring(0, $path.LastIndexOf("gcc"))
            Write-Verbose "Use GCC at $($ccprefix)."
            break
        }
    }

    if (($null -eq $ccprefix) -and $AllowFallback)
    {
        $ccprefix="/usr/bin/aarch64-linux-gnu-"
        Write-Warning -Message "GCC not found, fallback to /usr/bin/aarch64-linux-gnu- prefix."
    }

    # Now it's not the fallback case
    if ($null -eq $ccprefix)
    {
        Write-Error -Message "GCC not found. Either Linaro or distro-GCC is needed for build."
    }

    $env:GCC5_AARCH64_PREFIX = $ccprefix
    return $ccprefix
}

function Test-GnuAarch64CrossCollectionVersionRequirements 
{
    $prefix = Get-GnuAarch64CrossCollectionPath -AllowFallback
    $cc = "$($prefix)gcc"
    $versionOutput = . $cc --version
    if (($null -eq $versionOutput) -or ($versionOutput.Length -lt 1))
    {
        Write-Error -Message "GCC AArch64 toolchain is malfunctioned"
        return $false
    }

    # Match line one using RegEx
    $match = [regex]::Match($versionOutput[0], '[0-9]+\.[0-9]+\.[0-9]+')
    if ($match.Success -eq $true)
    {
        # Load version .NET assembly.
        $gccVersion = [System.Version]::Parse($match.Value)
        # Need at least GCC5
        return ($gccVersion.Major -ge 5)
    }

    # Fallback
    return $false
}

# Exports
Export-ModuleMember -Function Get-GnuAarch64CrossCollectionPath
Export-ModuleMember -Function Test-GnuAarch64CrossCollectionVersionRequirements
