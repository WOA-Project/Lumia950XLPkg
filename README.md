# EDK2 Implementation for Lumia 950 XL

## What's this?

This package demonstrates an AArch64 UEFI implementation for hacked Lumia 950 XL. 
Currently it is able to boot Windows 10 ARM64 (with a minor patch).
Booting Linux is also possible if seven cores are disabled in ACPI tables (due to the TZ implementation).

## Build

If you are familar with EDK2, you don't need to use my build script.

- Checkout a copy of [EDK2](https://github.com/tianocore/edk2), then checkout this repository under EDK2's worktree.
- Install [Linaro AArch64 GCC toolchains](http://releases.linaro.org/components/toolchain/binaries/), my build
script uses `gcc-linaro-7.2.1-2017.11`. Then untar them. I place everything under `/opt` directory, so I have 
directories like `/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin`. If you placed it somewhere else, modify build
scripts.
- Copy `rundbbuild.sh` in `Tools` directory to your EDK2 worktree root directory.
- Export variable `GCC5_AARCH64_PREFIX` to your GCC directory with prefix (e.g. `/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin/aarch64-elf-`)
- Enable `-mfix-cortex-a53-835769 -mfix-cortex-a53-843419` flags in GCC will make you day better.
- Start build: `. rundbbuild.sh --950xl --development`

## Run

It requires a kickstarter to run. Public details soon.

## Patch for booting Windows ARM64

Go to your EDK2 worktree, find `MdePkg/Include/AArch64/ProcessorBind.h`:

	- #define RUNTIME_PAGE_ALLOCATION_GRANULARITY (0x10000)
	+ #define RUNTIME_PAGE_ALLOCATION_GRANULARITY (0x1000)

This issue will be addressed with improved memory allocation in the future.

## TZ Implementation Notes

Qualcomm Snapdragon MSM8992/MSM8994 implements a subset of [PSCI interface](http://infocenter.arm.com/help//topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf) for multi-processor startup. However, required
commands like `PSCI_SYSTEM_OFF` and `PSCI_SYSTEM_RESET` are not implemented. Hence we use PMIC to shutdown
platform (there's a bug in RT that will be fixed) instead of PSCI. Additionally, 8992/8994 uses HVC call for
PSCI commands instead of SMC call. It seems that Linux encounters some troubles during MP startup. 

## Linux Notes

The ACPI tables are copied from stock Windows Phone FFU, hence these device IDs are likely not be recognized by Linux.
A proper device tree is still required to boot Linux. If you are interested, you are welcome to contribute.

## Acknowledgements

- [EFIDroid Project](http://efidroid.org)

## License

All code except drivers in `GPLDriver` directory are licensed under BSD 2-Clause. 
GPL Drivers are licensed under GPLv2 license.
