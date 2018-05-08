/* Minimum DSDT for WoA boot */

DefinitionBlock ("DSDT.aml", "DSDT", 5, "LMNL", "EDK2", 2)
{
	Scope (\_SB_)
    {
        /* 8 CPU Cores */
		Device (CPU0)
        {
		    Name (_HID, "ACPI0007")
            Name (_UID, 0x0)
			Method (_STA)
            {
                Return(0xf)
            }
        }

        Device (CPU1)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x1)
			Method (_STA)
            {
                Return(0xf)
            }
        }

        Device (CPU2)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x2)
			Method (_STA)
            {
                Return(0xf)
            }
        }

        Device (CPU3)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x3)
			Method (_STA)
            {
                Return(0xf)
            }
        }

        Device (CPU4)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x4)
			Method (_STA)
            {
                Return(0xf)
            }
        }

        Device (CPU5)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x5)
			Method (_STA)
            {
                Return(0xf)
            }
        }

        Device (CPU6)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x6)
			Method (_STA)
            {
                Return(0xf)
            }
        }

        Device (CPU7)
        {
            Name (_HID, "ACPI0007")
            Name (_UID, 0x7)
			Method (_STA)
            {
                Return(0xf)
            }
        }

		/* Internal eMMC */
		Device (SDC1)
        {
            Name (_HID, "QCOM24BF")
            Name (_CID, "ACPI\QCOM24BF")
            Name (_UID, 0)
            Name (_CCA, 0)

            Method (_CRS, 0, NotSerialized)
            {
                Name (RBUF, ResourceTemplate ()
                {
                    Memory32Fixed(ReadWrite, 0xF9824900, 0x00000200)
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive,,,)
                    {
                        0x0000009B,
                    }
                })
                Return (RBUF)
            }

            Device (EMMC) 
			{
                Method (_ADR) 
				{
                    Return (8)
                }
 
                Method (_RMV) 
				{
                    Return (0)
                }
            }

            Method (_DIS)
            {
            }

            Method (_STA)
            {
                Return (0x0F)
            }
        }

	}
}