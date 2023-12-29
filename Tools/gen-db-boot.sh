#!/bin/bash
~/sources/skales/mkbootimg --kernel=./Build/Dragonboard810-AARCH64/DEBUG_GCC5/FV/MSM8994_EFI.fd --cmdline="" --output=./Build/Dragonboard810-AARCH64/DEBUG_GCC5/FV/UEFI.boot.img --base 0x00000000 --pagesize=2048 --dt=./Lumia950XLPkg/Resources/DeviceTree/Dragonboard.dt.img
