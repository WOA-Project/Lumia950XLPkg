/**
@file  EFIPCIeInit.h
@brief Charger UEFI Protocol definitions.
*/
/*=============================================================================
Copyright (c) 2011-2014 Qualcomm Technologies, Incorporated.
All rights reserved.
Qualcomm Technologies, Confidential and Proprietary.
=============================================================================*/

/*=============================================================================
EDIT HISTORY


when       who     what, where, why
--------   ---     -----------------------------------------------------------
12/03/14   tselvam Added support for De-initialization
06/16/14   hk      created

=============================================================================*/
#ifndef __EFIPCIEINIT_H__
#define __EFIPCIEINIT_H__

#define EFI_QCOM_PCIE_INIT_GUID \
    { 0x93B80004, 0x9FB3, 0x11d4, { 0x9A, 0x3A, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D } }
#define QCOM_PCIE_INIT_PROTOCOL EFI_QCOM_PCIE_INIT_GUID

typedef struct _QCOM_PCIE_PROTOCOL QCOM_PCIE_PROTOCOL;

/*===========================================================================
FUNCTION DEFINITIONS
===========================================================================*/
typedef 
EFI_STATUS 
(EFIAPI * EFI_PCIE_INIT) 
(IN QCOM_PCIE_PROTOCOL *This);

typedef
UINT32 
(EFIAPI * EFI_PCI_RP_READ_CONFIG_32) 
(IN UINT32 rpIndex, IN UINT32 offset);

typedef
EFI_STATUS 
(EFIAPI * EFI_PCIE_DEINIT) 
(void);


/*===========================================================================
PROTOCOL INTERFACE
===========================================================================*/

struct _QCOM_PCIE_PROTOCOL {
	EFI_PCIE_INIT   PCIeInitHardware;
	EFI_PCI_RP_READ_CONFIG_32 PCIe_Config_Read_32;
	EFI_PCIE_DEINIT PCIeDeInitHardware;
};

#endif  /* __EFIPCIEINIT_H__ */
