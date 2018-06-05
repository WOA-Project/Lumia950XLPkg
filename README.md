# EDK2 Implementation for Lumia 950 & Lumia 950 XL
[![Build Status](https://travis-ci.com/imbushuo/Lumia950XLPkg.svg?branch=msm8994)](https://travis-ci.com/imbushuo/Lumia950XLPkg)

## What's this?

This package demonstrates an AArch64 UEFI implementation for hacked Lumia 950 and Lumia 950 XL. 
Currently it is able to boot Windows 10 ARM64 (with a minor patch).
Booting Linux is also possible (see the note).

## What can you do?

I am too busy to write an average-user instruction. So if you are interested in, you are welcome to
contribute to an easy instruction for all Lumia 950 XL users.

Or you can buy me a coffee: [PayPal](https://www.paypal.com/paypalme/imbushuo).

## Build

If you are familar with EDK2, you don't need to use my build script.

- Checkout a copy of [EDK2](https://github.com/tianocore/edk2), then checkout this repository under EDK2's worktree.
- Commit `0e2a5749d89c96e3e17ea458365d2e5296c807e2` absoultely works for you.
- Install [Linaro AArch64 GCC toolchains](http://releases.linaro.org/components/toolchain/binaries/), my build
script uses `gcc-linaro-7.2.1-2017.11`. Then untar them. I place everything under `/opt` directory, so I have 
directories like `/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin`. If you placed it somewhere else, modify build
scripts.
- Copy `rundbbuild.sh` in `Tools` directory to your EDK2 worktree root directory.
- Export variable `GCC5_AARCH64_PREFIX` to your GCC directory with prefix (e.g. `/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin/aarch64-elf-`)
- Start build: `. rundbbuild.sh --950xl --development`

## Run

Per UEFI specification, ARM32 UEFI cannot boot ARM64 binaries directly. 
To run this UEFI build on Lumia 950 XL, the following procedure is required:

- Check out [Boot Shim](https://github.com/imbushuo/boot-shim). This Boot Manager Application 
implements a simple ELF loader for the kickstarter (LK).
- Check out [LK](https://github.com/imbushuo/lk). You need `msm8994-test-2` branch. Build 
MSM8994 target, then you will get a `emmc_appsboot.mbn` file.
- Re-partition your Lumia 950XL, shrink `Data` partition, create a new partition with GPT name
`boot`, and another `uefi_vars` (reserved for further use). 4MB for both partition is okay. Tools
like `cgdisk` is recommended.
- Place `emmc_appsboot.mbn` in the WP EFIESP root directory, copy Boot Shim EFI appliction
to the EFIESP partition, create a new BCD entry for it.
- Boot to this BCD entry, you should enter Android Fastboot mode.
- Connect to your computer, flash the build: `fastboot flash boot UEFI.elf`
- Continue to UEFI: `fastboot continue`

To re-flash UEFI, press volume down until Fastboot device shows up (like flashing Android phones).

## Patch for booting Windows ARM64

You don't need the patch anymore. If you have applied the patch, you should change it back in 
`MdePkg/Include/AArch64/ProcessorBind.h`.

## TZ Implementation Notes

Qualcomm Snapdragon MSM8992/MSM8994 implements a subset of [PSCI interface](http://infocenter.arm.com/help//topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf) for multi-processor startup. However, required
commands like `PSCI_SYSTEM_OFF` and `PSCI_SYSTEM_RESET` are not implemented. Hence we use PMIC to shutdown
platform (there's a bug in RT that will be fixed) instead of PSCI. Additionally, 8992/8994 uses HVC call for
PSCI commands instead of SMC call. It seems that Linux encounters some troubles during MP startup. 

## Linux Notes

The ACPI tables are copied from stock Windows Phone FFU, hence these device IDs are likely not be recognized by Linux.

To get started, starts with the device tree of Qualcomm MSM8994 MTP. The repository `devicetree-rebasing` with 
DT content from Android Linux Kernel is sufficient for DT development. To boot with device tree, add it in your
GRUB configuration:
	
	devicetree /lumia-950-xl.dtb
	linux /vmlinuz ..... acpi=no

PSCI partially works. If you want to use PSCI for multi-processor startup, add the following code to your DT:

	psci {
		compatible	= "arm,psci-0.2";
		method		= "hvc";
	};

And use `psci` for core-enable method. I think the MSM8994 Cortex ACC method works if you are working on a kernel
for MSM8994, but I have not tested it yet.

## Acknowledgements

- [EFIDroid Project](http://efidroid.org)
- Andrei Warkentin and his [RaspberryPiPkg](https://github.com/andreiw/RaspberryPiPkg)
- Sarah Purohit

## License

All code except drivers in `GPLDriver` directory are licensed under BSD 2-Clause. 
GPL Drivers are licensed under GPLv2 license.
