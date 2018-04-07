#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/InterruptsLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/HardwareInterrupt.h>

typedef struct {
  UINTN                 Vector;
  int_handler           Handler;
  VOID                  *Arg;
  LIST_ENTRY            Link;
} HANDLER_ENTRY;

STATIC EFI_HARDWARE_INTERRUPT_PROTOCOL *mInterrupt = NULL;
STATIC LIST_ENTRY mHandlers = INITIALIZE_LIST_HEAD_VARIABLE(mHandlers);

STATIC HANDLER_ENTRY* GetInterruptHandlerEntry (UINTN Vector)
{
  LIST_ENTRY        *Link;
  HANDLER_ENTRY     *Entry;
  EFI_TPL        OriginalTPL;

  OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);

  for (Link = mHandlers.ForwardLink; Link != &mHandlers; Link = Link->ForwardLink) {
    Entry = BASE_CR (Link, HANDLER_ENTRY, Link);
    if (Entry->Vector == Vector) {
      gBS->RestoreTPL (OriginalTPL);
      return Entry;
    }
  }

  gBS->RestoreTPL (OriginalTPL);

  return NULL;
}

VOID
EFIAPI
InterruptsLibIrqHandler (
  IN  HARDWARE_INTERRUPT_SOURCE   Source,
  IN  EFI_SYSTEM_CONTEXT          SystemContext
  )
{
  EFI_TPL     OriginalTPL;

  // get handler entry
  HANDLER_ENTRY* Entry = GetInterruptHandlerEntry ((UINTN)Source);
  ASSERT (Entry);

  OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);

  // call handler, ignore the return value because we don't support threads
  Entry->Handler (Entry->Arg);

  gBS->RestoreTPL (OriginalTPL);

  // signal eoi
  mInterrupt->EndOfInterrupt (mInterrupt, Source);
}


INTN mask_interrupt(UINTN Vector)
{
  EFI_STATUS Status = mInterrupt->DisableInterruptSource (mInterrupt, Vector);
  ASSERT_EFI_ERROR (Status);
  return Status==EFI_SUCCESS?0:-1;
}

INTN unmask_interrupt(UINTN Vector)
{
  EFI_STATUS Status = mInterrupt->EnableInterruptSource (mInterrupt, Vector);
  ASSERT_EFI_ERROR (Status);
  return Status==EFI_SUCCESS?0:-1;
}

VOID register_int_handler(UINTN Vector, int_handler Handler, VOID *Arg)
{
  EFI_STATUS     Status;
  EFI_TPL        OriginalTPL;
  HANDLER_ENTRY  *Entry;

  // make sure, we didn't register a handler already
  Entry = GetInterruptHandlerEntry (Vector);
  ASSERT (Entry == NULL);

  // allocate entry structure
  Entry = AllocateZeroPool (sizeof (HANDLER_ENTRY));
  if (Entry == NULL) {
    ASSERT (FALSE);
    return;
  }

  // fill struct and insert
  Entry->Vector     = Vector;
  Entry->Handler    = Handler;
  Entry->Arg        = Arg;
  InsertTailList (&mHandlers, &Entry->Link);

  OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);

  // register interrupt source
  Status = mInterrupt->RegisterInterruptSource (mInterrupt, Vector, InterruptsLibIrqHandler);
  ASSERT_EFI_ERROR (Status);

  // disable interrupt source
  mask_interrupt (Vector);

  gBS->RestoreTPL (OriginalTPL);
}


RETURN_STATUS
EFIAPI
InterruptsLibConstructor (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gHardwareInterruptProtocolGuid, NULL, (VOID **)&mInterrupt);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
