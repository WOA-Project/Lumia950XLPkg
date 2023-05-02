/** @file

  Copyright (c) 2011 - 2020, Arm Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PREPI_H_
#define _PREPI_H_

#include <PiPei.h>

#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>

EFI_STATUS
EFIAPI
MemoryPeim(IN EFI_PHYSICAL_ADDRESS UefiMemoryBase, IN UINT64 UefiMemorySize);

EFI_STATUS
EFIAPI
PlatformPeim(VOID);

// Either implemented by PrePiLib or by MemoryInitPei
VOID BuildMemoryTypeInformationHob(VOID);

EFI_STATUS
EFIAPI
QGicPeim(VOID);

VOID QGicCpuInit(VOID);
VOID QgicCpuInitSecondary(VOID);

UINTN EFIAPI ArmGicAcknowledgeInterrupt(
    IN UINTN GicInterruptInterfaceBase, OUT UINTN *InterruptId);
VOID EFIAPI ArmGicEnableInterruptInterface(IN INTN GicInterruptInterfaceBase);

VOID EFIAPI
ArmGicV2EndOfInterrupt(IN UINTN GicInterruptInterfaceBase, IN UINTN Source);

VOID EFIAPI
ArmGicEndOfInterrupt(IN UINTN GicInterruptInterfaceBase, IN UINTN Source);

UINTN EFIAPI ArmGicGetMaxNumInterrupts(IN INTN GicDistributorBase);

#pragma pack(1)
typedef struct {
  /* First 2KB is reserved for OS */
  UINT32 ProcessorId;
  UINT32 Reserved;
  UINT64 JumpAddress;
  UINT8  OsReserved[2032];
  /* Next 2KB is reserved for firmware */
  UINT64 El2JumpFlag;
} EFI_PROCESSOR_MAILBOX, *PEFI_PROCESSOR_MAILBOX;
#pragma pack()

#endif /* _PREPI_H_ */
