// NPA Shim
// This is intended to maintain compatibility with Qualcomm binary module.
// Do not invoke if not necessary.

#ifndef __NPA_SHIM_H__
#define __NPA_SHIM_H__

#define EFI_NPA_SHIM_PROTOCOL_REVISION 0x0000000000010002

#define EFI_NPA_SHIM_PROTOCOL_GUID \
  { 0x79d6c870, 0x725e, 0x489e, { 0xa0, 0xa1, 0x27, 0xe7, 0xa5, 0xd0, 0xcb, 0x35 } }

/* Client work models; clients may only be of one of these types */
typedef enum
{
	NPA_NO_CLIENT = 0x7fffffff, /* Do not create a client */
	NPA_CLIENT_RESERVED1 = (1 << 0), /* Reserved client type */
	NPA_CLIENT_RESERVED2 = (1 << 1), /* Reserved client type */
	NPA_CLIENT_CUSTOM1 = (1 << 2), /* Custom client types - can be */
	NPA_CLIENT_CUSTOM2 = (1 << 3), /* customized by resource */
	NPA_CLIENT_CUSTOM3 = (1 << 4),
	NPA_CLIENT_CUSTOM4 = (1 << 5),
	/* Standard client types */
	NPA_CLIENT_REQUIRED = (1 << 6), /* Request must honor request */
	NPA_CLIENT_ISOCHRONOUS = (1 << 7), /* Placeholder for isochronous work model */
	NPA_CLIENT_IMPULSE = (1 << 8), /* Poke the resource - node defines the impulse response */
	NPA_CLIENT_LIMIT_MAX = (1 << 9), /* Set maximum value for the resource */
	NPA_CLIENT_VECTOR = (1 << 10), /* Vector request */
	NPA_CLIENT_SUPPRESSIBLE = (1 << 11), /* Suppressible Scalar request */
	NPA_CLIENT_SUPPRESSIBLE_VECTOR = ((1 << 12) | NPA_CLIENT_VECTOR), /* Suppressible Vector request */
	/* Another block of custom client types; can be customized by resource */
	NPA_CLIENT_CUSTOM5 = (1 << 13),
	NPA_CLIENT_CUSTOM6 = (1 << 14)
} npa_client_type;

typedef void* npa_client_handle;
typedef unsigned int npa_resource_state;

typedef EFI_STATUS(EFIAPI* EFI_NPA_CREATE_SYNC_CLIENT_EX)(
	const char        *resource_name,
	const char        *client_name,
	npa_client_type   client_type,
	unsigned int      client_value,
	void              *client_ref,
	npa_client_handle *client_handle
);

typedef EFI_STATUS(EFIAPI* EFI_NPA_COMPLETE_REQUEST)(
	npa_client_handle client
);

typedef EFI_STATUS(EFIAPI* EFI_NPA_DESTROY_CLIENT)(
	npa_client_handle client
);

typedef EFI_STATUS(EFIAPI* EFI_NPA_SCALAR_REQUEST)(
	npa_client_handle   client,
	npa_resource_state  state
);

typedef struct _EFI_NPA_PROTOCOL {
	UINT64								Revision;
	void*								NpaInit;
	EFI_NPA_CREATE_SYNC_CLIENT_EX		CreateSyncClientEx;
	EFI_NPA_COMPLETE_REQUEST			CompleteRequest;
	void*								DefineNodeCb;
	EFI_NPA_SCALAR_REQUEST				ScalarRequest;
	void*								ResourceAvailableCb;
	void*								RemoteDefineResourceCb;
	void*								RemoteResourceLocalAggregationFcn;
	void*								RemoteResourceLocalAggregationNoInitialRequestDriverFcn;
	void*								RemoteResourceRemoteAggregationDriverFcn;
	void*								RemoteResourceAvailable;
	void*								DalEventCallback;
	void*								ResourcesAvailableCb;
	void*								QueryResourceAvailable;
	EFI_NPA_DESTROY_CLIENT				DestroyClient;
	// Not support then
} EFI_NPA_PROTOCOL;

#endif