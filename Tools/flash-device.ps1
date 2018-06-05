#!/usr/bin/pwsh
# This script flashes device.
$Command = Get-Command -Name fastboot -ErrorAction SilentlyContinue
if ($Command -eq $null)
{
    Write-Error "Android Fastboot tool is not found."
    return
}

# Check content
$BuildContent = Get-ChildItem -Path Build/Lumia950XL-AARCH64/**/UEFI.elf -Recurse -ErrorAction SilentlyContinue
if ($BuildContent -eq $null)
{
    Write-Error "FD Payload is not found."
    return
}

# Flash image
fastboot flash boot "$($BuildContent.FullName)"
fastboot continue
