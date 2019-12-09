Update 12/09/19: PPTT table has been added and ASL C file has
been provided. However, the table has been pre-compiled and 
bundled as blobs.

The AML files included here are compiled from the ASL sources.
They need to be regenerated manually every time the ASLs change.

One exception is SSDT table. The generation procedure will be 
invoked with UEFI builds. Make sure you have iASL in the build 
environment.

All tables except the DSDTs are known to compile properly with
iasl version 20190108, by simply calling "iasl <name of table>.asl"

Unfortunately DSDT is compiled by this version into AML containing
new opcodes from ACPI 6.x, which Windows doesn't understand.
As a result, DSDT instead needs to be compiled using Microsoft's
own asl.exe compiler, available as part of the Windows 10 WDK.
(Version 17134 if the WDK is known to work well for this purpose.)

To compile DSDT, you need to call asl.exe like this:
asl.exe /Fo=DSDT.aml DSDT.asl
This is known to generate AML code that Windows is able to parse.