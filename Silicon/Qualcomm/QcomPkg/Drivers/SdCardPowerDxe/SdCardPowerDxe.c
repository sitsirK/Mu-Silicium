/**
  SD Card Slot Power Driver.

  On SoCs up to SM8250, SdccDxe looks for an SD Card without powering the Slot
  first, and nothing else in the Firmware does it either. The Card is then never
  found, even though the Device Configuration defines a PMIC Client for exactly
  that Purpose. (The SdccDxe of later SoCs requests its Client itself.)

  This Driver requests that Client, so that the Slot has Power by the Time the
  Boot Manager signals the SD Card Init Event.

  Only add this Driver to Devices that have an SD Card Slot: on a Board without
  one, the Regulators behind the Client may be wired to something else.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/EFINpa.h>

EFI_STATUS
EFIAPI
PowerSdCardSlot (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS        Status;
  EFI_NPA_PROTOCOL *mNpaProtocol;
  VOID             *ClientHandle = NULL;
  CONST CHAR8      *ResourceName = FixedPcdGetPtr (PcdSdCardNpaResource);

  // Locate NPA Protocol
  Status = gBS->LocateProtocol (&gEfiNpaProtocolGuid, NULL, (VOID *)&mNpaProtocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Locate NPA Protocol! Status = %r\n", Status));
    return Status;
  }

  // Verify Protocol Functions
  if (mNpaProtocol->CreateSyncClientEx == NULL || mNpaProtocol->IssueRequiredRequest == NULL) {
    DEBUG ((EFI_D_ERROR, "The NPA Protocol does not have the expected Functions!\n"));
    return EFI_UNSUPPORTED;
  }

  // Create SD Card Power Client
  Status = mNpaProtocol->CreateSyncClientEx (ResourceName, "SdCardPower", NPA_CLIENT_REQUIRED, 0, NULL, &ClientHandle);
  if (EFI_ERROR (Status) || ClientHandle == NULL) {
    DEBUG ((EFI_D_ERROR, "Failed to Create a Client on \"%a\"! Status = %r\n", ResourceName, Status));
    return EFI_NOT_FOUND;
  }

  // Power On SD Card Slot
  Status = mNpaProtocol->IssueRequiredRequest (ClientHandle, FixedPcdGet32 (PcdSdCardNpaMode));
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Failed to Power On the SD Card Slot! Status = %r\n", Status));
    return Status;
  }

  return EFI_SUCCESS;
}
