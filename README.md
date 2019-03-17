# EDK2 Implementation for Lumia 950, Lumia 950 XL and Hapanero

## For final users

You can download the latest UEFI build by clicking the Azure Pipelines icon below. Click Artifacts button in Azure Pipelines, then download UEFI.elf in ELF directory.

[![Build Status (Visual Studio Team Services)](https://dev.azure.com/LumiaWoA/Lumia950XLPkg/_apis/build/status/Lumia950XLPkg%20CI%20build?branchName=msm8994)](https://dev.azure.com/LumiaWoA/Lumia950XLPkg/_build/latest?definitionId=1&branchName=msm8994)

## What's this?

This package demonstrates an AArch64 UEFI implementation for hacked Lumia 950, Lumia 950 XL and Hapanero. Currently, it is able to boot Windows 10 ARM64, as well as Linux. See notes below for more details.

**Please be aware that MSM8992 is likely less supported due to lack of testing device.**

## Support Status
Applicable to all supported targets unless noted.

- Low-speed I/O: I2C, GPIO, SPMI and Pinmux(TLMM).
- Power Management: PMIC and RPM.
- High-speed I/O for firmware and HLOS use: eMMC (HS200), PCI Express (Firmware-configured, HLOS Only, x2 Lane)
- Peripherals: Touchscreen (I2C), side-band buttons (TLMM GPIO and PMIC GPIO)
- Display FrameBuffer

## What can you do?

I am too busy to write an average-user instruction. So, if you are interested in, you are welcome to
contribute to an easy instruction for all Lumia 950 XL users.

Or you can buy me a coffee: [PayPal](https://www.paypal.com/paypalme/imbushuo).

## Build

If you are familiar with EDK2, you don't need to use my build script.

- Checkout a copy of [EDK2](https://github.com/tianocore/edk2). Switch to `UDK2018` branch. Commit `49fa59e82e4c6ea798f65fc4e5948eae63ad6e07` absolutely works for you.
- Checkout this repository under EDK2's worktree.
- Install [Linaro AArch64 GCC toolchains](http://releases.linaro.org/components/toolchain/binaries/), my build
script uses `gcc-linaro-7.2.1-2017.11`. Then untar them. I place everything under `/opt` directory, so I have 
directories like `/opt/gcc-linaro-7.2.1-2017.11-x86_64_aarch64-elf/bin`. If you placed it somewhere else, modify build scripts.
- Run EDK2 BaseTools setup (`make -C BaseTools`).
- Copy `rundbbuild.sh` in `Tools` directory to your EDK2 worktree root directory.
- By default only MSM8994 target is built. To build all, set environment variable `BUILDALL`.
- Start build: `. rundbbuild.sh --950xl --development`

## WSL Build Notes

If you checked out the EDK2 repository under Windows and build it using WSL, you will have trouble locating BaseTools Python classes due to [file case sensitive behavior changes](https://blogs.msdn.microsoft.com/commandline/2018/02/28/per-directory-case-sensitivity-and-wsl/) in WSL. Run the following PowerShell script under EDK2 directory prior to build:

	Get-ChildItem .\BaseTools\ -Directory -Recurse | Foreach-Object { fsutil.exe file setCaseSensitiveInfo $_.FullName }

You only need to run it once.

## Run

Per UEFI specification, ARM32 UEFI cannot boot ARM64 binaries directly. A recent engineering
change removed the dependency of Little Kernel. LK can still boot it, but the path is untested.

To run this UEFI build on Lumia 950 XL, the following procedure is required:

- Check out [Boot Shim](https://github.com/imbushuo/boot-shim). This Boot Manager Application 
implements a simple ELF loader for the kickstarter (LK) with Secure Monitor Call for EL1 transition.
You will need branch `msm8994-aa64`.
- Place `UEFI.elf` in the WP EFIESP root directory, copy Boot Shim EFI application
to the EFIESP partition, create a new BCD entry for it.
- Select this boot entry to enter UEFI.

To re-flash UEFI, simply place new `UEFI.elf` in WP EFIESP root directory.

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

And use `psci` for core-enable method.

For MSM8994, PCI Express Root Port 1 is **firmware-initialized**. Hence it is not necessary to supply `qcom,pcie` in device tree. Instead, supply a firmware-initialized PCI bus device `pci-host-cam-generic`. ACPI MCFG table is supplied for your reference.

## Acknowledgements

- [EFIDroid Project](http://efidroid.org)
- Andrei Warkentin and his [RaspberryPiPkg](https://github.com/andreiw/RaspberryPiPkg)
- Sarah Purohit
- [Googulator](https://github.com/Googulator/)

## License

All code except drivers in `GPLDriver` directory are licensed under BSD 2-Clause. 
GPL Drivers are licensed under GPLv2 license.
