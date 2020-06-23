DefinitionBlock ("", "SSDT", 2, "MMO   ", "MSM8992 ", 0x00000011)
{
    External (_SB_.ABD_.AVBL, IntObj)
    External (_SB_.GIO0, DeviceObj)
    External (_SB_.GIO0.NFFO, OpRegionObj)
    External (_SB_.GIO0.NFPO, OpRegionObj)
    External (_SB_.I2C7, UnknownObj)
    External (_SB_.IC11, UnknownObj)
    External (_SB_.PEP0, UnknownObj)
    External (_SB_.PEP0.FLD0, UnknownObj)
    External (_SB_.PM02, UnknownObj)
    External (_SB_.SP10, UnknownObj)
    External (DBFL, UnknownObj)
    External (ESNL, UnknownObj)
    External (GIO0, DeviceObj)

    Scope (\_SB)
    {
        Device (SDC2)
        {
            Name (_DEP, Package (One)  // _DEP: Dependencies
            {
                \_SB.PEP0
            })
            Name (_HID, "QCOM2466")  // _HID: Hardware ID
            Name (_CID, "ACPIQCOM2466")  // _CID: Compatible ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_CCA, Zero)  // _CCA: Cache Coherency Attribute
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (RBUF, ResourceTemplate ()
                {
                    Memory32Fixed (ReadWrite,
                        0xF98A4900,         // Address Base
                        0x00000200,         // Address Length
                        )
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
                    {
                        0x0000009D,
                    }
                })
                Return (RBUF) /* \_SB_.SDC2._CRS.RBUF */
            }

            Device (EMMC)
            {
                Method (_ADR, 0, NotSerialized)  // _ADR: Address
                {
                    Return (0x08)
                }

                Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
                {
                    Return (Zero)
                }
            }

            Method (_DIS, 0, NotSerialized)  // _DIS: Disable Device
            {
            }

            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                Return (0x0F)
            }
        }

        Device (EGP0)
        {
            Name (_HID, "MSHW1008")  // _HID: Hardware ID
            Name (_CID, "MSHW1008")  // _CID: Compatible ID
            Name (_UID, One)  // _UID: Unique ID
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (RBUF, ResourceTemplate ()
                {
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.PM01", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0638
                        }
                    GpioIo (Shared, PullUp, 0x1838, 0x0000, IoRestrictionNone,
                        "\\_SB.PM01", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0610
                        }
                    GpioIo (Shared, PullUp, 0x1838, 0x0000, IoRestrictionNone,
                        "\\_SB.PM01", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0041
                        }
                    GpioIo (Shared, PullUp, 0x1838, 0x0000, IoRestrictionNone,
                        "\\_SB.PM01", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0620
                        }
                    GpioIo (Shared, PullUp, 0x1838, 0x0000, IoRestrictionNone,
                        "\\_SB.PM01", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0618
                        }
                    GpioIo (Shared, PullDown, 0x1838, 0x0000, IoRestrictionNone,
                        "\\_SB.PM01", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0040
                        }
                    GpioIo (Shared, PullDown, 0x1838, 0x0000, IoRestrictionNone,
                        "\\_SB.PM01", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0608
                        }
                    GpioIo (Shared, PullUp, 0x1838, 0x0000, IoRestrictionInputOnly,
                        "\\_SB.PM01", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0668
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0064
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0034
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x002A
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x002A
                        }
                    GpioIo (Shared, PullDown, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x007D
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0016
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x004B
                        }
                    I2cSerialBusV2 (0x006C, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.IC11",
                        0x00, ResourceConsumer, , Exclusive,
                        )
                })
                Return (RBUF) /* \_SB_.EGP0._CRS.RBUF */
            }
        }

        Device (APS1)
        {
            Name (_HID, "MSHW1016")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_DEP, Package (0x02)  // _DEP: Dependencies
            {
                \_SB.PEP0, 
                \_SB.I2C7
            })
            Method (PRIM, 0, NotSerialized)
            {
                Name (RBUF, Buffer (One)
                {
                     0x01                                             // .
                })
                Return (RBUF) /* \_SB_.APS1.PRIM.RBUF */
            }

            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (RBUF, ResourceTemplate ()
                {
                    I2cSerialBusV2 (0x0039, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.I2C7",
                        0x00, ResourceConsumer, , Exclusive,
                        )
                    GpioInt (Edge, ActiveLow, Exclusive, PullUp, 0x0000,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0028
                        }
                })
                Return (RBUF) /* \_SB_.APS1._CRS.RBUF */
            }
        }

        Device (SMDH)
        {
            Name (_HID, "MSHW100D")  // _HID: Hardware ID
        }

        Device (MCPU)
        {
            Name (_HID, "MSHW1014")  // _HID: Hardware ID
        }

        Device (HALL)
        {
            Name (_HID, "MSHW1015")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_DEP, Package (0x02)  // _DEP: Dependencies
            {
                \_SB.PEP0, 
                \_SB.GIO0
            })
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (RBUF, ResourceTemplate ()
                {
                    GpioInt (Edge, ActiveHigh, SharedAndWake, PullUp, 0x0000,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x002A
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x002A
                        }
                    GpioInt (Edge, ActiveHigh, SharedAndWake, PullUp, 0x0000,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x004B
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x004B
                        }
                })
                Return (RBUF) /* \_SB_.HALL._CRS.RBUF */
            }

            Name (PGID, Buffer (0x0A)
            {
                "\\_SB.HALL"
            })
            Name (DBUF, Buffer (DBFL){})
            CreateByteField (DBUF, Zero, STAT)
            CreateByteField (DBUF, 0x02, DVAL)
            CreateField (DBUF, 0x18, 0xA0, DEID)
            Method (_S1D, 0, NotSerialized)  // _S1D: S1 Device State
            {
                Return (0x03)
            }

            Method (_S2D, 0, NotSerialized)  // _S2D: S2 Device State
            {
                Return (0x03)
            }

            Method (_S3D, 0, NotSerialized)  // _S3D: S3 Device State
            {
                Return (0x03)
            }

            Method (_PS0, 0, NotSerialized)  // _PS0: Power State 0
            {
                DEID = Buffer (ESNL){}
                DVAL = Zero
                DEID = PGID /* \_SB_.HALL.PGID */
                If (\_SB.ABD.AVBL)
                {
                    \_SB.PEP0.FLD0 = DBUF /* \_SB_.HALL.DBUF */
                }
            }

            Method (_PS3, 0, NotSerialized)  // _PS3: Power State 3
            {
                DEID = Buffer (ESNL){}
                DVAL = 0x03
                DEID = PGID /* \_SB_.HALL.PGID */
                If (\_SB.ABD.AVBL)
                {
                    \_SB.PEP0.FLD0 = DBUF /* \_SB_.HALL.DBUF */
                }
            }
        }

        Device (CESP)
        {
            Name (_HID, "MSHW1010")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
        }

        Device (NFC1)
        {
            Name (_HID, "PNP0547")  // _HID: Hardware ID
            Name (_CID, Package (0x02)  // _CID: Compatible ID
            {
                "PN547", 
                "ACPIPN547"
            })	
            Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
            {
                I2cSerialBusV2 (0x0028, ControllerInitiated, 0x00061A80,
                    AddressingMode7Bit, "\\_SB.I2C6",
                    0x00, ResourceConsumer, , Exclusive,
                    )
                GpioInt (Edge, ActiveHigh, Exclusive, PullDown, 0x0000,
                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                    )
                    {   // Pin list
                        0x001D
                    }
            })
            Name (NFCP, ResourceTemplate ()
            {
                GpioIo (Exclusive, PullNone, 0x0000, 0x0000, IoRestrictionNone,
                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                    )
                    {   // Pin list
                        0x001E
                    }
            })
            Scope (GIO0)
            {
                OperationRegion (NFPO, GeneralPurposeIo, Zero, One)
            }

            Field (\_SB.GIO0.NFPO, ByteAcc, NoLock, Preserve)
            {
                Connection (\_SB.NFC1.NFCP), 
                MGPE,   1
            }

            Method (POON, 0, NotSerialized)
            {
                MGPE = One
            }

            Method (POOF, 0, NotSerialized)
            {
                MGPE = Zero
            }

            Name (NFCF, ResourceTemplate ()
            {
                GpioIo (Exclusive, PullNone, 0x0000, 0x0000, IoRestrictionNone,
                    "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                    )
                    {   // Pin list
                        0x005E
                    }
            })
            Scope (GIO0)
            {
                OperationRegion (NFFO, GeneralPurposeIo, Zero, One)
            }

            Field (\_SB.GIO0.NFFO, ByteAcc, NoLock, Preserve)
            {
                Connection (\_SB.NFC1.NFCF), 
                MGFE,   1
            }

            Method (FWON, 0, NotSerialized)
            {
                MGFE = One
            }

            Method (FWOF, 0, NotSerialized)
            {
                MGFE = Zero
            }

            Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
            {
                Debug = "Method NFC _DSM begin"
                If ((Arg0 == ToUUID ("a2e7f6c4-9638-4485-9f12-6b4e20b60d63")))
                {
                    If ((Arg2 == Zero))
                    {
                        Debug = "Method NFC _DSM QUERY"
                        If ((Arg1 == One))
                        {
                            Return (Buffer (One)
                            {
                                 0x0F                                             // .
                            })
                        }
                    }

                    If ((Arg2 == One))
                    {
                        Debug = "Method NFC _DSM SETFWMODE"
                        If ((Arg3 == One))
                        {
                            \_SB.NFC1.POOF ()
                            \_SB.NFC1.FWON ()
                            Sleep (One)
                            \_SB.NFC1.POON ()
                            Sleep (0x14)
                        }

                        If ((Arg3 == Zero))
                        {
                            \_SB.NFC1.POOF ()
                            \_SB.NFC1.FWOF ()
                            Sleep (One)
                            \_SB.NFC1.POON ()
                            Sleep (0x14)
                        }
                    }

                    If ((Arg2 == 0x02))
                    {
                        Debug = "Method NFC _DSM SETPOWERMODE"
                        If ((Arg3 == One))
                        {
                            \_SB.NFC1.POON ()
                            Sleep (0x14)
                        }

                        If ((Arg3 == Zero))
                        {
                            \_SB.NFC1.POOF ()
                            Sleep (0x14)
                        }
                    }

                    If ((Arg2 == 0x03))
                    {
                        Debug = "Method NFC _DSM EEPROM Config"
                        Return (Buffer (0x03CA)
                        {
                            /* 0000 */  0x54, 0x61, 0x6C, 0x6B, 0x6D, 0x61, 0x6E, 0x00,  // Talkman.
                            /* 0008 */  0x00, 0x00, 0x00, 0x20, 0x08, 0x01, 0x22, 0x00,  // ... ..".
                            /* 0010 */  0x02, 0x01, 0x01, 0x03, 0x01, 0x11, 0x04, 0x01,  // ........
                            /* 0018 */  0x01, 0x06, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x11,  // ........
                            /* 0020 */  0x04, 0xCD, 0x67, 0x22, 0x01, 0x12, 0x01, 0x00,  // ..g"....
                            /* 0028 */  0x40, 0x01, 0x01, 0x41, 0x01, 0x04, 0x42, 0x01,  // @..A..B.
                            /* 0030 */  0x19, 0x43, 0x01, 0x00, 0x61, 0x01, 0x00, 0x5E,  // .C..a..^
                            /* 0038 */  0x01, 0x01, 0xCD, 0x01, 0x0F, 0xEC, 0x01, 0x01,  // ........
                            /* 0040 */  0xED, 0x01, 0x00, 0x0D, 0x03, 0x04, 0x43, 0x20,  // ......C 
                            /* 0048 */  0x0D, 0x03, 0x04, 0xFF, 0x05, 0x0D, 0x06, 0x06,  // ........
                            /* 0050 */  0x44, 0xA3, 0x90, 0x03, 0x00, 0x0D, 0x06, 0x06,  // D.......
                            /* 0058 */  0x30, 0xCF, 0x00, 0x08, 0x00, 0x0D, 0x06, 0x06,  // 0.......
                            /* 0060 */  0x2F, 0x8F, 0x05, 0x80, 0x0C, 0x0D, 0x04, 0x06,  // /.......
                            /* 0068 */  0x03, 0x00, 0x6E, 0x0D, 0x03, 0x06, 0x43, 0xA0,  // ..n...C.
                            /* 0070 */  0x0D, 0x06, 0x06, 0x42, 0x00, 0x00, 0xFF, 0xFF,  // ...B....
                            /* 0078 */  0x0D, 0x06, 0x06, 0x41, 0x80, 0x00, 0x00, 0x00,  // ...A....
                            /* 0080 */  0x0D, 0x03, 0x06, 0x37, 0x18, 0x0D, 0x03, 0x06,  // ...7....
                            /* 0088 */  0x16, 0x00, 0x0D, 0x03, 0x06, 0x15, 0x00, 0x0D,  // ........
                            /* 0090 */  0x06, 0x06, 0xFF, 0x05, 0x00, 0x00, 0x00, 0x0D,  // ........
                            /* 0098 */  0x06, 0x08, 0x44, 0x00, 0x00, 0x00, 0x00, 0x0D,  // ..D.....
                            /* 00A0 */  0x06, 0x20, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x0D,  // . J.....
                            /* 00A8 */  0x06, 0x20, 0x42, 0x88, 0x10, 0xFF, 0xFF, 0x0D,  // . B.....
                            /* 00B0 */  0x03, 0x20, 0x16, 0x00, 0x0D, 0x03, 0x20, 0x15,  // . .... .
                            /* 00B8 */  0x00, 0x0D, 0x06, 0x22, 0x44, 0x22, 0x00, 0x02,  // ..."D"..
                            /* 00C0 */  0x00, 0x0D, 0x06, 0x22, 0x2D, 0x50, 0x44, 0x0C,  // ..."-PD.
                            /* 00C8 */  0x00, 0x0D, 0x04, 0x32, 0x03, 0x40, 0x3D, 0x0D,  // ...2.@=.
                            /* 00D0 */  0x06, 0x32, 0x42, 0xF8, 0x10, 0xFF, 0xFF, 0x0D,  // .2B.....
                            /* 00D8 */  0x03, 0x32, 0x16, 0x00, 0x0D, 0x03, 0x32, 0x15,  // .2....2.
                            /* 00E0 */  0x01, 0x0D, 0x03, 0x32, 0x0D, 0x22, 0x0D, 0x03,  // ...2."..
                            /* 00E8 */  0x32, 0x14, 0x22, 0x0D, 0x06, 0x32, 0x4A, 0x30,  // 2."..2J0
                            /* 00F0 */  0x07, 0x01, 0x1F, 0x0D, 0x06, 0x34, 0x2D, 0x24,  // .....4-$
                            /* 00F8 */  0x77, 0x0C, 0x00, 0x0D, 0x06, 0x34, 0x34, 0x00,  // w....44.
                            /* 0100 */  0x00, 0xE4, 0x03, 0x0D, 0x06, 0x34, 0x44, 0x21,  // .....4D!
                            /* 0108 */  0x00, 0x02, 0x00, 0x0D, 0x06, 0x35, 0x44, 0x21,  // .....5D!
                            /* 0110 */  0x00, 0x02, 0x00, 0x0D, 0x06, 0x38, 0x4A, 0x53,  // .....8JS
                            /* 0118 */  0x07, 0x01, 0x1B, 0x0D, 0x06, 0x38, 0x42, 0x68,  // .....8Bh
                            /* 0120 */  0x10, 0xFF, 0xFF, 0x0D, 0x03, 0x38, 0x16, 0x00,  // .....8..
                            /* 0128 */  0x0D, 0x03, 0x38, 0x15, 0x00, 0x0D, 0x06, 0x3A,  // ..8....:
                            /* 0130 */  0x2D, 0x15, 0x47, 0x0D, 0x00, 0x0D, 0x06, 0x3C,  // -.G....<
                            /* 0138 */  0x4A, 0x52, 0x07, 0x01, 0x1B, 0x0D, 0x06, 0x3C,  // JR.....<
                            /* 0140 */  0x42, 0x68, 0x10, 0xFF, 0xFF, 0x0D, 0x03, 0x3C,  // Bh.....<
                            /* 0148 */  0x16, 0x00, 0x0D, 0x03, 0x3C, 0x15, 0x00, 0x0D,  // ....<...
                            /* 0150 */  0x06, 0x3E, 0x2D, 0x15, 0x47, 0x0D, 0x00, 0x0D,  // .>-.G...
                            /* 0158 */  0x06, 0x40, 0x42, 0xF0, 0x10, 0xFF, 0xFF, 0x0D,  // .@B.....
                            /* 0160 */  0x03, 0x40, 0x0D, 0x02, 0x0D, 0x03, 0x40, 0x14,  // .@....@.
                            /* 0168 */  0x02, 0x0D, 0x06, 0x40, 0x4A, 0x12, 0x07, 0x00,  // ...@J...
                            /* 0170 */  0x00, 0x0D, 0x03, 0x40, 0x16, 0x00, 0x0D, 0x03,  // ...@....
                            /* 0178 */  0x40, 0x15, 0x00, 0x0D, 0x06, 0x42, 0x2D, 0x15,  // @....B-.
                            /* 0180 */  0x47, 0x0D, 0x00, 0x0D, 0x06, 0x46, 0x44, 0x21,  // G....FD!
                            /* 0188 */  0x00, 0x02, 0x00, 0x0D, 0x06, 0x46, 0x2D, 0x05,  // .....F-.
                            /* 0190 */  0x47, 0x0E, 0x00, 0x0D, 0x06, 0x44, 0x4A, 0x33,  // G....DJ3
                            /* 0198 */  0x07, 0x01, 0x07, 0x0D, 0x06, 0x44, 0x42, 0x88,  // .....DB.
                            /* 01A0 */  0x10, 0xFF, 0xFF, 0x0D, 0x03, 0x44, 0x16, 0x00,  // .....D..
                            /* 01A8 */  0x0D, 0x03, 0x44, 0x15, 0x00, 0x0D, 0x06, 0x4A,  // ..D....J
                            /* 01B0 */  0x44, 0x22, 0x00, 0x02, 0x00, 0x0D, 0x06, 0x4A,  // D".....J
                            /* 01B8 */  0x2D, 0x05, 0x37, 0x0C, 0x00, 0x0D, 0x06, 0x48,  // -.7....H
                            /* 01C0 */  0x4A, 0x33, 0x07, 0x01, 0x07, 0x0D, 0x06, 0x48,  // J3.....H
                            /* 01C8 */  0x42, 0x88, 0x10, 0xFF, 0xFF, 0x0D, 0x03, 0x48,  // B......H
                            /* 01D0 */  0x16, 0x00, 0x0D, 0x03, 0x48, 0x15, 0x00, 0x0D,  // ....H...
                            /* 01D8 */  0x06, 0x4E, 0x44, 0x22, 0x00, 0x02, 0x00, 0x0D,  // .ND"....
                            /* 01E0 */  0x06, 0x4E, 0x2D, 0x05, 0x37, 0x0C, 0x00, 0x0D,  // .N-.7...
                            /* 01E8 */  0x06, 0x4C, 0x4A, 0x33, 0x07, 0x01, 0x07, 0x0D,  // .LJ3....
                            /* 01F0 */  0x06, 0x4C, 0x42, 0x88, 0x10, 0xFF, 0xFF, 0x0D,  // .LB.....
                            /* 01F8 */  0x03, 0x4C, 0x16, 0x00, 0x0D, 0x03, 0x4C, 0x15,  // .L....L.
                            /* 0200 */  0x00, 0x0D, 0x06, 0x52, 0x44, 0x22, 0x00, 0x02,  // ...RD"..
                            /* 0208 */  0x00, 0x0D, 0x06, 0x52, 0x2D, 0x05, 0x25, 0x0C,  // ...R-.%.
                            /* 0210 */  0x00, 0x0D, 0x06, 0x50, 0x42, 0x90, 0x10, 0xFF,  // ...PB...
                            /* 0218 */  0xFF, 0x0D, 0x06, 0x50, 0x4A, 0x11, 0x0F, 0x01,  // ...PJ...
                            /* 0220 */  0x07, 0x0D, 0x03, 0x50, 0x16, 0x00, 0x0D, 0x03,  // ...P....
                            /* 0228 */  0x50, 0x15, 0x00, 0x0D, 0x06, 0x56, 0x2D, 0x05,  // P....V-.
                            /* 0230 */  0x9E, 0x0C, 0x00, 0x0D, 0x06, 0x56, 0x44, 0x22,  // .....VD"
                            /* 0238 */  0x00, 0x02, 0x00, 0x0D, 0x06, 0x5C, 0x2D, 0x05,  // .....\-.
                            /* 0240 */  0x69, 0x0C, 0x00, 0x0D, 0x06, 0x5C, 0x44, 0x21,  // i....\D!
                            /* 0248 */  0x00, 0x02, 0x00, 0x0D, 0x06, 0x54, 0x42, 0x88,  // .....TB.
                            /* 0250 */  0x10, 0xFF, 0xFF, 0x0D, 0x06, 0x54, 0x4A, 0x33,  // .....TJ3
                            /* 0258 */  0x07, 0x01, 0x07, 0x0D, 0x03, 0x54, 0x16, 0x00,  // .....T..
                            /* 0260 */  0x0D, 0x03, 0x54, 0x15, 0x00, 0x0D, 0x06, 0x5A,  // ..T....Z
                            /* 0268 */  0x42, 0x90, 0x10, 0xFF, 0xFF, 0x0D, 0x06, 0x5A,  // B......Z
                            /* 0270 */  0x4A, 0x31, 0x07, 0x01, 0x07, 0x0D, 0x03, 0x5A,  // J1.....Z
                            /* 0278 */  0x16, 0x00, 0x0D, 0x03, 0x5A, 0x15, 0x00, 0x0D,  // ....Z...
                            /* 0280 */  0x06, 0x98, 0x2F, 0xAF, 0x05, 0x80, 0x0F, 0x0D,  // ../.....
                            /* 0288 */  0x06, 0x9A, 0x42, 0x00, 0x00, 0xFF, 0xFF, 0x0D,  // ..B.....
                            /* 0290 */  0x06, 0x30, 0x44, 0xA3, 0x90, 0x03, 0x00, 0x0D,  // .0D.....
                            /* 0298 */  0x06, 0x6C, 0x44, 0xA3, 0x90, 0x03, 0x00, 0x0D,  // .lD.....
                            /* 02A0 */  0x06, 0x6C, 0x30, 0xCF, 0x00, 0x08, 0x00, 0x0D,  // .l0.....
                            /* 02A8 */  0x06, 0x6C, 0x2F, 0x8F, 0x05, 0x80, 0x0C, 0x0D,  // .l/.....
                            /* 02B0 */  0x06, 0x70, 0x2F, 0x8F, 0x05, 0x80, 0x12, 0x0D,  // .p/.....
                            /* 02B8 */  0x06, 0x70, 0x30, 0xCF, 0x00, 0x08, 0x00, 0x0D,  // .p0.....
                            /* 02C0 */  0x06, 0x74, 0x2F, 0x8F, 0x05, 0x80, 0x12, 0x0D,  // .t/.....
                            /* 02C8 */  0x06, 0x74, 0x30, 0xDF, 0x00, 0x07, 0x00, 0x0D,  // .t0.....
                            /* 02D0 */  0x06, 0x78, 0x2F, 0x1F, 0x06, 0x80, 0x01, 0x0D,  // .x/.....
                            /* 02D8 */  0x06, 0x78, 0x30, 0x3F, 0x00, 0x04, 0x00, 0x0D,  // .x0?....
                            /* 02E0 */  0x06, 0x78, 0x44, 0xA2, 0x90, 0x03, 0x00, 0x0D,  // .xD.....
                            /* 02E8 */  0x03, 0x78, 0x47, 0x00, 0x0D, 0x06, 0x7C, 0x2F,  // .xG...|/
                            /* 02F0 */  0xAF, 0x05, 0x80, 0x0F, 0x0D, 0x06, 0x7C, 0x30,  // ......|0
                            /* 02F8 */  0xCF, 0x00, 0x07, 0x00, 0x0D, 0x06, 0x7C, 0x44,  // ......|D
                            /* 0300 */  0xA3, 0x90, 0x03, 0x00, 0x0D, 0x06, 0x7D, 0x30,  // ......}0
                            /* 0308 */  0xCF, 0x00, 0x08, 0x00, 0x0D, 0x06, 0x80, 0x2F,  // ......./
                            /* 0310 */  0xAF, 0x05, 0x80, 0x90, 0x0D, 0x06, 0x80, 0x44,  // .......D
                            /* 0318 */  0xA3, 0x90, 0x03, 0x00, 0x0D, 0x06, 0x84, 0x2F,  // ......./
                            /* 0320 */  0xAF, 0x05, 0x80, 0x92, 0x0D, 0x06, 0x84, 0x44,  // .......D
                            /* 0328 */  0xA3, 0x90, 0x03, 0x00, 0x0D, 0x06, 0x88, 0x2F,  // ......./
                            /* 0330 */  0x7F, 0x04, 0x80, 0x10, 0x0D, 0x06, 0x88, 0x30,  // .......0
                            /* 0338 */  0x5F, 0x00, 0x16, 0x00, 0x0D, 0x03, 0x88, 0x47,  // _......G
                            /* 0340 */  0x00, 0x0D, 0x06, 0x88, 0x44, 0xA1, 0x90, 0x03,  // ....D...
                            /* 0348 */  0x00, 0x0D, 0x03, 0x10, 0x43, 0x20, 0x0D, 0x06,  // ....C ..
                            /* 0350 */  0x6A, 0x42, 0xF8, 0x10, 0xFF, 0xFF, 0x0D, 0x03,  // jB......
                            /* 0358 */  0x6A, 0x16, 0x00, 0x0D, 0x03, 0x6A, 0x15, 0x01,  // j....j..
                            /* 0360 */  0x0D, 0x06, 0x6A, 0x4A, 0x30, 0x0F, 0x01, 0x1F,  // ..jJ0...
                            /* 0368 */  0x0D, 0x06, 0x8C, 0x42, 0x88, 0x10, 0xFF, 0xFF,  // ...B....
                            /* 0370 */  0x0D, 0x06, 0x8C, 0x4A, 0x33, 0x07, 0x01, 0x07,  // ...J3...
                            /* 0378 */  0x0D, 0x03, 0x8C, 0x16, 0x00, 0x0D, 0x03, 0x8C,  // ........
                            /* 0380 */  0x15, 0x00, 0x0D, 0x06, 0x92, 0x42, 0x90, 0x10,  // .....B..
                            /* 0388 */  0xFF, 0xFF, 0x0D, 0x06, 0x92, 0x4A, 0x31, 0x07,  // .....J1.
                            /* 0390 */  0x01, 0x07, 0x0D, 0x03, 0x92, 0x16, 0x00, 0x0D,  // ........
                            /* 0398 */  0x03, 0x92, 0x15, 0x00, 0x0D, 0x06, 0x0A, 0x30,  // .......0
                            /* 03A0 */  0xCF, 0x00, 0x08, 0x00, 0x0D, 0x06, 0x0A, 0x2F,  // ......./
                            /* 03A8 */  0x8F, 0x05, 0x80, 0x0C, 0x0D, 0x03, 0x0A, 0x48,  // .......H
                            /* 03B0 */  0x10, 0x0D, 0x06, 0x0A, 0x44, 0xA3, 0x90, 0x03,  // ....D...
                            /* 03B8 */  0x00, 0x0D, 0x03, 0x06, 0x48, 0x19, 0x0D, 0x03,  // ....H...
                            /* 03C0 */  0x0C, 0x48, 0x19, 0x0D, 0x03, 0x00, 0x40, 0x03   // .H....@.
                        })
                    }
                }
            }
        }

        Device (SEC)
        {
            Name (_HID, "MSHW1001")  // _HID: Hardware ID
        }

        Device (LACT)
        {
            Name (_HID, "MSHW101C")  // _HID: Hardware ID
            Method (MODE, 0, NotSerialized)
            {
                Name (RBUF, Buffer (One)
                {
                     0x01                                             // .
                })
                Return (RBUF) /* \_SB_.LACT.MODE.RBUF */
            }
        }

        Device (LSTP)
        {
            Name (_HID, "MSHW101D")  // _HID: Hardware ID
            Method (MODE, 0, NotSerialized)
            {
                Name (RBUF, Buffer (One)
                {
                     0x02                                             // .
                })
                Return (RBUF) /* \_SB_.LSTP.MODE.RBUF */
            }
        }

        Device (LGES)
        {
            Name (_HID, "MSHW101E")  // _HID: Hardware ID
            Method (MODE, 0, NotSerialized)
            {
                Name (RBUF, Buffer (One)
                {
                     0x03                                             // .
                })
                Return (RBUF) /* \_SB_.LGES.MODE.RBUF */
            }
        }

        Device (LSAC)
        {
            Name (_HID, "MSHW101F")  // _HID: Hardware ID
            Method (MODE, 0, NotSerialized)
            {
                Name (RBUF, Buffer (One)
                {
                     0x64                                             // d
                })
                Return (RBUF) /* \_SB_.LSAC.MODE.RBUF */
            }
        }

        Device (LASD)
        {
            Name (_DEP, Package (0x01)  // _DEP: Dependencies
            {
                \_SB.LACT
            })
            Name (_HID, "MSHW1020")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Method (PRIM, 0, NotSerialized)
            {
                Name (RBUF, Buffer (One)
                {
                     0x01                                             // .
                })
                Return (RBUF) /* \_SB_.LASD.PRIM.RBUF */
            }
        }

        Device (LPSD)
        {
            Name (_DEP, Package (0x01)  // _DEP: Dependencies
            {
                \_SB.LSTP
            })
            Name (_HID, "MSHW1021")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Method (PRIM, 0, NotSerialized)
            {
                Name (RBUF, Buffer (One)
                {
                     0x01                                             // .
                })
                Return (RBUF) /* \_SB_.LPSD.PRIM.RBUF */
            }
        }

        Device (CPSW)
        {
            Name (_HID, "LUMI0001")  // _HID: Hardware ID
            Name (_UID, One)  // _UID: Unique ID
            Name (_DEP, Package (0x04)  // _DEP: Dependencies
            {
                \_SB.PEP0, 
                \_SB.PM02, 
                \_SB.SP10, 
                \_SB.GIO0
            })
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (RBUF, ResourceTemplate ()
                {
                    SpiSerialBusV2 (0x0000, PolarityLow, FourWireMode, 0x08,
                        ControllerInitiated, 0x004C4B40, ClockPolarityLow,
                        ClockPhaseFirst, "\\_SB.SP10",
                        0x00, ResourceConsumer, , Exclusive,
                        RawDataBuffer (0x06)  // Vendor Data
                        {
                            0x00, 0x00, 0x00, 0x01, 0x00, 0x00
                        })
                    GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.PM02", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0620
                        }
                    GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.PM02", 0x00, ResourceConsumer, ,
                        RawDataBuffer (0x04)  // Vendor Data
                        {
                            0x00, 0x13, 0xFF, 0xFF
                        })
                        {   // Pin list
                            0x0638
                        }
                    GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.PM02", 0x00, ResourceConsumer, ,
                        RawDataBuffer (0x04)  // Vendor Data
                        {
                            0x00, 0x13, 0xFF, 0xFF
                        })
                        {   // Pin list
                            0x0640
                        }
                    GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.PM02", 0x00, ResourceConsumer, ,
                        RawDataBuffer (0x04)  // Vendor Data
                        {
                            0x00, 0x13, 0xFF, 0xFF
                        })
                        {   // Pin list
                            0x0648
                        }
                    GpioInt (Edge, ActiveBoth, ExclusiveAndWake, PullUp, 0x0000,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0016
                        }
                    GpioInt (Edge, ActiveLow, Exclusive, PullUp, 0x0000,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x005F
                        }
                    GpioIo (Shared, PullUp, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.PM02", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0618
                        }
                    GpioInt (Edge, ActiveHigh, Exclusive, PullUp, 0x0000,
                        "\\_SB.PM02", 0x00, ResourceConsumer, ,
                        RawDataBuffer (0x04)  // Vendor Data
                        {
                            0x21, 0x01, 0x02, 0x1A
                        })
                        {   // Pin list
                            0x1002
                        }
                    GpioInt (Edge, ActiveHigh, Exclusive, PullUp, 0x0000,
                        "\\_SB.PM02", 0x00, ResourceConsumer, ,
                        RawDataBuffer (0x04)  // Vendor Data
                        {
                            0x20, 0x01, 0x02, 0x1A
                        })
                        {   // Pin list
                            0x1001
                        }
                })
                Return (RBUF) /* \_SB_.CPSW._CRS.RBUF */
            }

            Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
            {
                ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */, 
                Package (0x01)
                {
                    Package (0x02)
                    {
                        "symbolic-identifiers", 
                        Package (0x1A)
                        {
                            Zero, 
                            "SPI0", 
                            One, 
                            One, 
                            0x02, 
                            0x02, 
                            0x03, 
                            0x03, 
                            0x04, 
                            0x04, 
                            0x05, 
                            0x05, 
                            0x06, 
                            0x05, 
                            0x07, 
                            0x06, 
                            0x08, 
                            0x06, 
                            0x09, 
                            0x07, 
                            0x0A, 
                            0x07, 
                            0x0B, 
                            0x08, 
                            0x0C, 
                            0x08
                        }
                    }
                }
            })
            Name (PGID, Buffer (0x0A)
            {
                "\\_SB.CPSW"
            })
            Name (DBUF, Buffer (DBFL){})
            CreateByteField (DBUF, Zero, STAT)
            CreateByteField (DBUF, 0x02, DVAL)
            CreateField (DBUF, 0x18, 0xA0, DEID)
            Method (_S1D, 0, NotSerialized)  // _S1D: S1 Device State
            {
                Return (0x03)
            }

            Method (_S2D, 0, NotSerialized)  // _S2D: S2 Device State
            {
                Return (0x03)
            }

            Method (_S3D, 0, NotSerialized)  // _S3D: S3 Device State
            {
                Return (0x03)
            }

            Method (_PS0, 0, NotSerialized)  // _PS0: Power State 0
            {
                DEID = Buffer (ESNL){}
                DVAL = Zero
                DEID = PGID /* \_SB_.CPSW.PGID */
                If (\_SB.ABD.AVBL)
                {
                    \_SB.PEP0.FLD0 = DBUF /* \_SB_.CPSW.DBUF */
                }
            }

            Method (_PS3, 0, NotSerialized)  // _PS3: Power State 3
            {
                DEID = Buffer (ESNL){}
                DVAL = 0x03
                DEID = PGID /* \_SB_.CPSW.PGID */
                If (\_SB.ABD.AVBL)
                {
                    \_SB.PEP0.FLD0 = DBUF /* \_SB_.CPSW.DBUF */
                }
            }
        }

        Device (HDDP)
        {
            Name (_HID, "MSHW1007")  // _HID: Hardware ID
            Name (_UID, Zero)  // _UID: Unique ID
            Name (_DEP, Package (0x04)  // _DEP: Dependencies
            {
                \_SB.PEP0, 
                \_SB.IC11, 
                \_SB.GIO0, 
                \_SB.PM02
            })
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (RBUF, ResourceTemplate ()
                {
                    I2cSerialBusV2 (0x0038, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.IC11",
                        0x00, ResourceConsumer, , Exclusive,
                        )
                    I2cSerialBusV2 (0x0039, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.IC11",
                        0x00, ResourceConsumer, , Exclusive,
                        )
                    I2cSerialBusV2 (0x003D, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.IC11",
                        0x00, ResourceConsumer, , Exclusive,
                        )
                    I2cSerialBusV2 (0x003F, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.IC11",
                        0x00, ResourceConsumer, , Exclusive,
                        )
                    I2cSerialBusV2 (0x0040, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.IC11",
                        0x00, ResourceConsumer, , Exclusive,
                        )
                    GpioInt (Edge, ActiveLow, Exclusive, PullUp, 0x0000,
                        "\\_SB.GIO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0029
                        }
                    GpioIo (Exclusive, PullNone, 0x0000, 0x0000, IoRestrictionNone,
                        "\\_SB.PM02", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list
                            0x0630
                        }
                })
                Return (RBUF) /* \_SB_.HDDP._CRS.RBUF */
            }

            Name (PGID, Buffer (0x0A)
            {
                "\\_SB.HDDP"
            })
            Name (DBUF, Buffer (DBFL){})
            CreateByteField (DBUF, Zero, STAT)
            CreateByteField (DBUF, 0x02, DVAL)
            CreateField (DBUF, 0x18, 0xA0, DEID)
            Method (_S1D, 0, NotSerialized)  // _S1D: S1 Device State
            {
                Return (0x03)
            }

            Method (_S2D, 0, NotSerialized)  // _S2D: S2 Device State
            {
                Return (0x03)
            }

            Method (_S3D, 0, NotSerialized)  // _S3D: S3 Device State
            {
                Return (0x03)
            }

            Method (_PS0, 0, NotSerialized)  // _PS0: Power State 0
            {
                DEID = Buffer (ESNL){}
                DVAL = Zero
                DEID = PGID /* \_SB_.HDDP.PGID */
                If (\_SB.ABD.AVBL)
                {
                    \_SB.PEP0.FLD0 = DBUF /* \_SB_.HDDP.DBUF */
                }
            }

            Method (_PS3, 0, NotSerialized)  // _PS3: Power State 3
            {
                DEID = Buffer (ESNL){}
                DVAL = 0x03
                DEID = PGID /* \_SB_.HDDP.PGID */
                If (\_SB.ABD.AVBL)
                {
                    \_SB.PEP0.FLD0 = DBUF /* \_SB_.HDDP.DBUF */
                }
            }
        }

        Device (TCCT)
        {
            Name (_HID, "MSHW100C")  // _HID: Hardware ID
        }
    }
}

