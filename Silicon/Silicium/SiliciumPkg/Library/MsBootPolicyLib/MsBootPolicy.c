/**
  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/MsBootPolicyLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/LoadedImage.h>

//
// Parameter of the "SD Card" Boot Option (see MsBootOptionsLib).
//
// MsBootPolicy only looks at the first Character of its Parameter, so "SD" runs
// the same Boot Sequence as "SSD" ("Internal Storage"). Which of the two the
// User picked is told apart here, per Device.
//
#define SD_CARD_BOOT_PARAMETER "SD"

BOOLEAN
EFIAPI
MsBootPolicyLibIsSettingsBoot ()
{
  return FALSE;
}

BOOLEAN
EFIAPI
MsBootPolicyLibIsAltBoot ()
{
  return FALSE;
}

EFI_STATUS
EFIAPI
MsBootPolicyLibClearBootRequests ()
{
  return EFI_SUCCESS;
}

STATIC
BOOLEAN
IsSdCardBoot ()
{
  EFI_STATUS                 Status;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

  // Locate Loaded Image Protocol of the Running Application
  Status = gBS->HandleProtocol (gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID *)&LoadedImage);
  if (EFI_ERROR (Status) || LoadedImage->LoadOptions == NULL) {
    return FALSE;
  }

  // Compare Boot Option Parameter
  if (LoadedImage->LoadOptionsSize != sizeof (SD_CARD_BOOT_PARAMETER)) {
    return FALSE;
  }

  return CompareMem (LoadedImage->LoadOptions, SD_CARD_BOOT_PARAMETER, sizeof (SD_CARD_BOOT_PARAMETER)) == 0;
}

STATIC
BOOLEAN
IsDevicePathSdCard (IN EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  EFI_STATUS             Status;
  EFI_HANDLE             Handle;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;

  // USB Drives are not SD Cards
  if (MsBootPolicyLibIsDevicePathUsb (DevicePath)) {
    return FALSE;
  }

  // Locate Block IO Handle of the Device
  Status = gBS->LocateDevicePath (&gEfiBlockIoProtocolGuid, &DevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  // Get Block IO Protocol from Handle
  Status = gBS->HandleProtocol (Handle, &gEfiBlockIoProtocolGuid, (VOID *)&BlockIo);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  // Internal Storage (UFS, eMMC) is Fixed Media, an SD Card is Removable
  return BlockIo->Media->RemovableMedia;
}

BOOLEAN
EFIAPI
MsBootPolicyLibIsDevicePathBootable (IN EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  // Check Device Path
  if (DevicePath == NULL || !IsDevicePathValid (DevicePath, 0)) {
    return FALSE;
  }

  // Check SD Card Boot Option Presence
  if (!FixedPcdGetBool (PcdSdCardBootOption)) {
    return TRUE;
  }

  // "SD Card" only Boots SD Cards, every other Boot Option never Boots them
  return IsDevicePathSdCard (DevicePath) == IsSdCardBoot ();
}

BOOLEAN
EFIAPI
MsBootPolicyLibIsDeviceBootable (IN EFI_HANDLE ControllerHandle)
{
  return MsBootPolicyLibIsDevicePathBootable (DevicePathFromHandle (ControllerHandle));
}

BOOLEAN
EFIAPI
MsBootPolicyLibIsDevicePathUsb (IN EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
  // Go thru each Device Path
  for (EFI_DEVICE_PATH_PROTOCOL *Node = DevicePath; !IsDevicePathEnd (Node); Node = NextDevicePathNode (Node)) {
    // Compare Device Path Types
    if ((DevicePathType (Node) == MESSAGING_DEVICE_PATH) && ((DevicePathSubType (Node) == MSG_USB_CLASS_DP) || (DevicePathSubType (Node) == MSG_USB_WWID_DP) || (DevicePathSubType (Node) == MSG_USB_DP))) {
      return TRUE;
    }
  }

  return FALSE;
}

EFI_STATUS
EFIAPI
MsBootPolicyLibGetBootSequence (
  OUT BOOT_SEQUENCE **BootSequence,
  IN  BOOLEAN         AltBootRequest)
{
  return EFI_UNSUPPORTED;
}
