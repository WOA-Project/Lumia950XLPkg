# EDK2 Implementation for Lumia 950, Lumia 950 XL and Hapanero

![Devices](https://raw.githubusercontent.com/WOA-Project/MSM8994-8992-NT-ARM64-Drivers/1911/assets/banner.png)

## For users

You can download the latest UEFI build by clicking the Azure Pipelines icon below. Click Artifacts button in Azure Pipelines, then download UEFI.elf in ELF directory.

[![Build Status (Visual Studio Team Services)](https://dev.azure.com/LumiaWoA/Lumia950XLPkg/_apis/build/status/Lumia950XLPkg%20CI%20build?branchName=master)](https://dev.azure.com/LumiaWoA/Lumia950XLPkg/_build/latest?definitionId=1&branchName=master)

## What's this?

This package demonstrates an AArch64 UEFI implementation for hacked Lumia 950, Lumia 950 XL and Hapanero. Currently it is able to boot Windows 10 ARM64 as well as various Linux distros. See notes below for more details.

**Please be aware that MSM8992 devices have limited support.**

## Support Status
Applicable to all supported targets unless noted.

- Low-speed I/O: I2C, SPI, GPIO, SPMI and Pinmux (TLMM).
- Power Management: PMIC and Resource Power Manager (RPM).
- High-speed I/O for firmware and HLOS: eMMC (SDR50 in firmware, HS200/HS400 in OS) and microSD (be aware that a few cards are unsupported), PCI Express (Firmware-configured, HLOS Only, x2 Lane)
- Peripherals: Touchscreen (QUP I2C), side-band buttons (TLMM GPIO and PMIC GPIO) and Lattice UC120 (iCE5LP2K) FPGA configuration
- Display FrameBuffer depends on stock Qualcomm UEFI for boostrapping, MDP is not fully implemented.

## What can you do?

I am too busy to write an average-user tutorial. So, if you are interested in, you are welcome to
contribute to an easy instruction for all Lumia 950 (XL) users.

Or you can buy me a coffee: [PayPal](https://www.paypal.com/paypalme/imbushuo).

## Build

If you are familiar with EDK2, you don't need to use my build script.

- Checkout a copy of [EDK2](https://github.com/tianocore/edk2). We are currently tracking the `master` branch. At least commit `1b6b4a83e1d85e48837068dfe409f5557b50d71d` works without further modification.
- Checkout this repository under EDK2's worktree.
- Install ACPI tools from your package manager or ACPICA website.
- Install `uuid-dev` and `python` (or equivalent package on your distribution).
- Install [Linaro AArch64 GCC toolchains](http://releases.linaro.org/components/toolchain/binaries/), my build
script uses `gcc-linaro-7.5.0-2019.12`. Then untar them. I place everything under `/opt` directory, so I have 
directories like `/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-elf/bin`. If you placed it somewhere else, modify build scripts. If you are macOS user, bootstrap the toolchain using ct-ng.
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
PSCI commands instead of SMC call.

## Linux Notes

The ACPI tables are copied from stock Windows Phone FFU, hence these device IDs are likely not be recognized by Linux.

To get started, starts with the device tree of Qualcomm MSM8994 MTP. The repository `devicetree-rebasing` with 
DT content from Android Linux Kernel is sufficient for DT development. To boot with device tree, add it in your
GRUB configuration:
	
	devicetree /lumia-950-xl.dtb
	linux /vmlinuz ..... acpi=no

PSCI partially works in EL1. If you want to use PSCI for multi-processor startup, add the following code to your DT:

	psci {
		compatible	= "arm,psci-0.2";
		method		= "hvc";
	};

And use `psci` for core-enable method. If you are using EL2 startup, use `spin-table` with `per_cpu_mailbox_addr + 0x8` as the release address.

For MSM8994, PCI Express Root Port 1 is **firmware-initialized**. Similarly, MSM8992 have PCI Express Root Port 0 initialized. Hence it is not necessary to supply `qcom,pcie` in device tree. Instead, supply a firmware-initialized PCI bus device `pci-host-ecam-generic`. ACPI MCFG table is supplied for your reference.

Note: interrupt is not configured in the example below (therefore ath10k will crash the system if loaded.)

	pci@f8800000 {
		compatible = "pci-host-ecam-generic";
		device_type = "pci";
		#address-cells = <0x3>;
		#size-cells = <0x2>;
		bus-range = <0x0 0x1>;
		#interrupt-cells = <0x1>;

		reg = <0xf8800000 0x200000>;
		ranges = <0x02000000 0x0 0xf8a00000 0xf8a00000 0x0 0x600000>;

		status = "okay";
	};

## Acknowledgements

- [EFIDroid Project](http://efidroid.org)
- Andrei Warkentin and his [RaspberryPiPkg](https://github.com/andreiw/RaspberryPiPkg)
- Sarah Purohit
- [Googulator](https://github.com/Googulator/)

## License

All code except drivers in `GPLDriver` directory are licensed under BSD 2-Clause. 
GPL Drivers are licensed under GPLv2 license.
