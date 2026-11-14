/**
  Qualcomm Node Power Architecture (NPA) Protocol.

  NpaDxe publishes this protocol. It is how the firmware's drivers ask for
  power: a driver creates a client on a named resource and issues a request
  for one of the modes that the device configuration defines for it
  (UFSDxe does this with "/pmic/client/ufs0", DisplayDxe with
  "/pmic/client/disp_prim", and so on).

  There is no public header for this protocol. The GUID and the two members
  declared here are taken from how those binary drivers call it; the members
  in between have not been identified and must not be called.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _EFI_NPA_H_
#define _EFI_NPA_H_

//
// Global GUID for the NPA Protocol
//
#define EFI_NPA_PROTOCOL_GUID { 0x79D6C870, 0x725E, 0x489E, { 0xA0, 0xA1, 0x27, 0xE7, 0xA5, 0xD0, 0xCB, 0x35 } }

//
// Declare forward reference to the NPA Protocol
//
typedef struct _EFI_NPA_PROTOCOL EFI_NPA_PROTOCOL;

//
// NPA Client Types
//
#define NPA_CLIENT_REQUIRED 0x40

/**
  This Function creates a synchronous Client on an NPA Resource.

  @param[in]  ResourceName                 - The Name of the Resource. (e.g. "/pmic/client/sd")
  @param[in]  ClientName                   - A Name for the new Client.
  @param[in]  ClientType                   - The Type of the Client.
  @param[in]  ClientValue                  - Client specific Value, 0 if unused.
  @param[in]  ClientRef                    - Client specific Reference, NULL if unused.
  @param[out] ClientHandle                 - The Handle of the new Client, NULL if the Resource does not exist.

  @return     Status                       - The EFI_STATUS returned by this Function.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_NPA_CREATE_SYNC_CLIENT_EX) (
  IN  CONST CHAR8  *ResourceName,
  IN  CONST CHAR8  *ClientName,
  IN  UINT32        ClientType,
  IN  UINT32        ClientValue,
  IN  VOID         *ClientRef,
  OUT VOID        **ClientHandle
  );

/**
  This Function issues a required Request on a Client.

  @param[in]  ClientHandle                 - The Handle of the Client.
  @param[in]  State                        - The requested State. (For a PMIC Client: its Mode Number)

  @return     Status                       - The EFI_STATUS returned by this Function.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_NPA_ISSUE_REQUIRED_REQUEST) (
  IN VOID   *ClientHandle,
  IN UINT32  State
  );

//
// Define Protocol Functions
//
struct _EFI_NPA_PROTOCOL {
  UINT64                          Revision;
  VOID                           *Reserved1;
  EFI_NPA_CREATE_SYNC_CLIENT_EX   CreateSyncClientEx;
  VOID                           *Reserved2;
  VOID                           *Reserved3;
  EFI_NPA_ISSUE_REQUIRED_REQUEST  IssueRequiredRequest;
};

extern EFI_GUID gEfiNpaProtocolGuid;

#endif /* _EFI_NPA_H_ */
