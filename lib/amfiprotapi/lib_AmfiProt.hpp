///-----------------------------------------------------------------------------
//                              AMFITECH APS
//                          ALL RIGHTS RESERVED
//-----------------------------------------------------------------------------

#ifndef LIB_AMFIPROT_H_
#define LIB_AMFIPROT_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <iostream>
#include <assert.h>
#include <stdio.h>

#ifdef _MSC_VER
#define __PACKED_STRUCT struct
#define __weak
#define __PACKED
#pragma pack(push, 1)
#define LIB_AMFIPROT_PACKED_PUSHED
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define __PACKED_STRUCT struct __attribute__((packed))
#define __weak
#define __PACKED __attribute__((packed))
#else
#define __PACKED_STRUCT struct __attribute__((packed))
#define __weak
#define __PACKED __attribute__((packed))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include "lib_Generic_Parameter.h"
#ifdef __cplusplus
}
#endif

#define sizeof_member(type, member) (sizeof(((type *)0)->member))

#ifndef AmfiProtMaxPacketLength
#define AmfiProtMaxPacketLength 62
#endif // AmfiProtMaxPacketLength
#define AmfiProtMaxPayloadLength (AmfiProtMaxPacketLength - sizeof(lib_AmfiProt_Header_t) - 1)
static_assert(AmfiProtMaxPacketLength <= UINT8_MAX, "AmfiProtMaxPacketLength larger than maximum allowed");

#define lib_AmfiProt_packetType_Mask (1 << 7 | 1 << 6)

//-----------------------------------------------------------------------------
// Type declarations
//-----------------------------------------------------------------------------
typedef struct lib_AmfiProt_FirmwareStart lib_AmfiProt_FirmwareStart_t;
typedef struct lib_AmfiProt_FirmwareEnd lib_AmfiProt_FirmwareEnd_t;
typedef struct lib_AmfiProt_FirmwareBinary lib_AmfiProt_FirmwareBinary_t;
typedef struct lib_AmfiProt_DeviceID lib_AmfiProt_DeviceID_t;
typedef struct lib_AmfiProt_RespondDeviceID lib_AmfiProt_RespondDeviceID_t;
typedef struct lib_AmfiProt_FirmwareVersion lib_AmfiProt_FirmwareVersion_t;
typedef struct lib_AmfiProt_FirmwareVersionPerID lib_AmfiProt_FirmwareVersionPerID_t;
typedef struct lib_AmfiProt_FirmwareVersionPerIDRequest lib_AmfiProt_FirmwareVersionPerIDRequest_t;
typedef struct lib_AmfiProt_DeviceName lib_AmfiProt_DeviceName_t;

typedef struct lib_AmfiProt_ConfigName lib_AmfiProt_ConfigName_t;
typedef struct lib_AmfiProt_ConfigNameRequest lib_AmfiProt_ConfigNameRequest_t;

typedef struct lib_AmfiProt_ConfigValue lib_AmfiProt_ConfigValue_t;
typedef struct lib_AmfiProt_ConfigValueRequest lib_AmfiProt_ConfigValueRequest_t;

typedef struct lib_AmfiProt_ConfigNameRequestUID lib_AmfiProt_ConfigNameRequestUID_t;

typedef struct lib_AmfiProt_ConfigNameUID_common lib_AmfiProt_ConfigNameUID_common_t;
typedef struct lib_AmfiProt_ConfigNameUID_protocol lib_AmfiProt_ConfigNameUID_protocol_t;
typedef struct lib_AmfiProt_ConfigNameUID_settings lib_AmfiProt_ConfigNameUID_settings_t;

typedef struct lib_AmfiProt_ConfigValueUID lib_AmfiProt_ConfigValueUID_t;
typedef struct lib_AmfiProt_ConfigValueUIDRequest lib_AmfiProt_ConfigValueUIDRequest_t;

typedef struct lib_AmfiProt_ConfigValueCount lib_AmfiProt_ConfigValueCount_t;
typedef struct lib_AmfiProt_ConfigValueCountRequest lib_AmfiProt_ConfigValueCountRequest_t;

typedef struct lib_AmfiProt_ConfigCategory lib_AmfiProt_ConfigCategory_t;
typedef struct lib_AmfiProt_ConfigCategoryRequest lib_AmfiProt_ConfigCategoryRequest_t;

typedef struct lib_AmfiProt_ConfigCategoryCount lib_AmfiProt_ConfigCategoryCount_t;

typedef struct lib_AmfiProt_LoadDefault lib_AmfiProt_LoadDefault_t;
typedef struct lib_AmfiProt_SaveAsDefault lib_AmfiProt_SaveAsDefault_t;
typedef struct lib_AmfiProt_DebugOutput lib_AmfiProt_DebugOutput_t;
typedef struct lib_AmfiProt_RebootDevice lib_AmfiProt_RebootDevice_t;
typedef struct libAmfiProt_TunnelData libAmfiProt_TunnelData_t;
typedef struct libAmfiProt_Invalid libAmfiProt_Invalid_t;

typedef struct lib_AmfiProt_ResetInfoRequest lib_AmfiProt_ResetInfoRequest_t;
typedef struct lib_AmfiProt_ResetInfoSummary lib_AmfiProt_ResetInfoSummary_t;
typedef struct lib_AmfiProt_ResetInfoChunk lib_AmfiProt_ResetInfoChunk_t;

typedef struct lib_AmfiProt_ProcedureSpecRequest lib_AmfiProt_ProcedureSpecRequest_t;
typedef struct lib_AmfiProt_ProcedureSpec lib_AmfiProt_ProcedureSpec_t;
typedef struct lib_AmfiProt_ProcedureRequest lib_AmfiProt_ProcedureRequest_t;
typedef struct lib_AmfiProt_ProcedureReply lib_AmfiProt_ProcedureReply_t;

typedef struct lib_AmfiProt_Header lib_AmfiProt_Header_t;
typedef struct lib_AmfiProt_Frame lib_AmfiProt_Frame_t;
typedef struct lib_AmfiProt_Handle lib_AmfiProt_Handle_t;

typedef enum
{
	lib_AmfiProt_packetType_NoAck = 0 << 6,
	lib_AmfiProt_packetType_RequestAck = 1 << 6,
	lib_AmfiProt_packetType_Ack = 2 << 6,
	lib_AmfiProt_packetType_Reply = 3 << 6,
} lib_AmfiProt_packetType_t;

typedef enum
{
	lib_AmfiProt_destination_PC = 0,
	lib_AmfiProt_destination_Broadcast = 0xFF,
} lib_AmfiProt_destination_t;

typedef enum
{
	lib_AmfiProt_PayloadID_RequestDeviceID = 0x00, // May be send on broadcast to scan for devices
	lib_AmfiProt_PayloadID_ReplyDeviceID = 0x01,
	lib_AmfiProt_PayloadID_SetTxID = 0x02, // May be send on broadcast, in case a device has an invalid/unknown TxID

	lib_AmfiProt_PayloadID_RequestFirmwareVersion = 0x03, // May be send on broadcast to scan for devices
	lib_AmfiProt_PayloadID_ReplyFirmwareVersion = 0x04,

	lib_AmfiProt_PayloadID_FirmwareStart = 0x05,
	lib_AmfiProt_PayloadID_FirmwareData = 0x06,
	lib_AmfiProt_PayloadID_FirmwareEnd = 0x07,

	lib_AmfiProt_PayloadID_RequestDeviceName = 0x08, // May be send on broadcast to scan for devices
	lib_AmfiProt_PayloadID_ReplyDeviceName = 0x09,

	lib_AmfiProt_PayloadID_RequestConfigurationValue = 0x0A,
	lib_AmfiProt_PayloadID_ReplyConfigurationValue = 0x0B,
	lib_AmfiProt_PayloadID_SetConfigurationValue = 0x0C,

	lib_AmfiProt_PayloadID_RequestConfigurationName = 0x0D,
	lib_AmfiProt_PayloadID_ReplyConfigurationName = 0x0E,

	lib_AmfiProt_PayloadID_LoadDefault = 0x0F,
	lib_AmfiProt_PayloadID_SaveAsDefault = 0x10,

	lib_AmfiProt_PayloadID_RequestConfigurationNameAndUID = 0x11,
	lib_AmfiProt_PayloadID_ReplyConfigurationNameAndUID = 0x12,

	lib_AmfiProt_PayloadID_RequestConfigurationValueUID = 0x13,
	lib_AmfiProt_PayloadID_ReplyConfigurationValueUID = 0x14,
	lib_AmfiProt_PayloadID_SetConfigurationValueUID = 0x15,

	lib_AmfiProt_PayloadID_RequestConfigurationCategory = 0x16,
	lib_AmfiProt_PayloadID_ReplyConfigurationCategory = 0x17,

	lib_AmfiProt_PayloadID_RequestConfigurationValueCount = 0x18,
	lib_AmfiProt_PayloadID_ReplyConfigurationValueCount = 0x19,

	lib_AmfiProt_PayloadID_RequestCategoryCount = 0x1A,
	lib_AmfiProt_PayloadID_ReplyCategoryCount = 0x1B,

	lib_AmfiProt_PayloadID_RequestFirmwareVersionPerID = 0x1C,
	lib_AmfiProt_PayloadID_ReplyFirmwareVersionPerID = 0x1D,

	lib_AmfiProt_PayloadID_RequestResetInfo = 0x1E,
	lib_AmfiProt_PayloadID_ReplyResetInfo = 0x1F,

	lib_AmfiProt_PayloadID_DebugOutput = 0x20,

	lib_AmfiProt_PayloadID_Reboot = 0x21,

	lib_AmfiProt_PayloadID_RequestEventLog = 0x22,
	lib_AmfiProt_PayloadID_ReplyEventLog = 0x23,

	lib_AmfiProt_PayloadID_ResetParameter = 0x24,

	lib_AmfiProt_PayloadID_RequestProcedureSpec = 0x30,
	lib_AmfiProt_PayloadID_ReplyProcedureSpec = 0x31,
	lib_AmfiProt_PayloadID_RequestProcedureCall = 0x32,
	lib_AmfiProt_PayloadID_ReplyProcedureCall = 0x33,

	lib_AmfiProt_PayloadID_TunneledData = 0x50,
	lib_AmfiProt_PayloadID_TunneledDataRaw = 0x51,

} lib_AmfiProt_PayloadID_t;

typedef enum
{
	libAmfiProt_PayloadType_Common = 0x00,
	libAmfiProt_PayloadType_Success = 0xF0,
	libAmfiProt_PayloadType_NotImplemented = 0xFD,
	libAmfiProt_PayloadType_Failure = 0xFE,
	libAmfiProt_PayloadType_InvalidRequest = 0xFF,
} lib_AmfiProt_PayloadType_t;

typedef enum
{
	libAmfiProt_StatusType_Success = 0xF0,
	libAmfiProt_StatusType_Timeout = 0xFC,
	libAmfiProt_StatusType_NotImplemented = 0xFD,
	libAmfiProt_StatusType_Failure = 0xFE,
	libAmfiProt_StatusType_InvalidRequest = 0xFF,
} lib_AmfiProt_StatusType_t;

typedef enum
{
	lib_AmfiProt_ConfigCategory_Force_All = 0xFE,
	lib_AmfiProt_ConfigCategory_All = 0xFF,
} lib_AmfiProt_ConfigCategoryNames_t;

typedef enum
{
	lib_AmfiProt_ResetInfoField_Summary = 0x00,
	lib_AmfiProt_ResetInfoField_File    = 0x01,
	lib_AmfiProt_ResetInfoField_Func    = 0x02,
	lib_AmfiProt_ResetInfoField_Expr    = 0x03,
	lib_AmfiProt_ResetInfoField_Last,
} lib_AmfiProt_ResetInfoField_t;

typedef enum
{
	lib_AmfiProt_ResetInfoRecord_None      = 0x00,
	lib_AmfiProt_ResetInfoRecord_HardFault = 0x01,
	lib_AmfiProt_ResetInfoRecord_Assert    = 0x02,
	lib_AmfiProt_ResetInfoRecord_Reset     = 0x03,  // Watchdog, brownout or illegal low-power entry
} lib_AmfiProt_ResetInfoRecord_t;

class lib_AmfiProt
{
  public:
	lib_AmfiProt();
	~lib_AmfiProt();

	bool lib_AmfiProt_Init(lib_AmfiProt_Handle_t *handle, uint8_t _deviceID);
	bool lib_AmfiProt_UpdateCRC(lib_AmfiProt_Frame_t *frame);
	bool lib_AmfiProt_EncodeFrame(lib_AmfiProt_Frame_t *frame, void const *pPayload, uint8_t length, uint8_t payloadType, uint8_t packetNumber, uint8_t destination, lib_AmfiProt_packetType_t packetType);
	bool lib_AmfiProt_EncodeAck(lib_AmfiProt_Frame_t *incomingFrame, lib_AmfiProt_Frame_t *outgoingFrame);
	bool lib_AmfiProt_DeserializeFrame(lib_AmfiProt_Frame_t *frame, void const *pData, uint8_t length);
	uint8_t lib_AmfiProt_FrameSize(lib_AmfiProt_Frame_t const *frame);
	void lib_AmfiProt_SetDeviceID(uint8_t _deviceID);

	void lib_AmfiProt_ProcessFrame(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle);

	virtual void libAmfiProt_handle_RequestProcedureSpec(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyProcedureSpec(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestProcedureCall(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyProcedureCall(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestDeviceID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RespondDeviceID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_SetTxID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestFirmwareVersion(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_FirmwareVersion(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_FirmwareStart(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_FirmwareData(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_FirmwareEnd(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestDeviceName(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyDeviceName(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestConfigurationValue(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyConfigurationValue(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_SetConfigurationValue(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestConfigurationName(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyConfigurationName(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_LoadDefault(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_SaveAsDefault(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestConfigurationNameAndUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ConfigurationNameAndUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestConfigurationValueUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ConfigurationValueUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_SetConfigurationValueUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestConfigurationCategory(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ConfigurationCategory(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestConfigurationValueCount(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ConfigurationValueCount(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestCategoryCount(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_CategoryCount(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_Reboot(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_DebugOutput(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ResetParameter(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestFirmwareVersionPerID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyFirmwareVersionPerID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_RequestResetInfo(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyResetInfo(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_AlternativeProcessing(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;

	virtual void libAmfiProt_ReplyInvalid(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;

	virtual void libAmfiProt_handle_Ack(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplySuccess(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyNotImplemented(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyFailure(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void libAmfiProt_handle_ReplyInvalidRequest(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;

  private:
	uint8_t deviceID;
};

//-----------------------------------------------------------------------------
// Variables and constants
//-----------------------------------------------------------------------------
struct lib_AmfiProt_Handle
{
	uint8_t deviceID;
};

__PACKED_STRUCT lib_AmfiProt_Header
{
	uint8_t length;		  // Length of payload (without crc)
	uint8_t packetType;	  // Bit 6-7: 0 = No Ack, 1 = Request Ack, 2 = Ack, 3 = Reply   Bit 0-5: Time to live for packet routing
	uint8_t packetNumber; // Sequentially increasing packet number, used when sending back ack
	uint8_t payloadType;  // Payload Type - 0: Standard functions (Application Name, Device UUID, Device TxID, Application configuration, Firmware update, etc.), 1+: Application specific
	uint8_t source;		  // 0: Reserved for USB connected PC
	uint8_t destination;  // 0: Reserved for USB connected PC, 255: Broadcast
	uint8_t headCRC;	  // 8 bit CRC of the header
};

struct lib_AmfiProt_Frame
{
	lib_AmfiProt_Header_t header;
	uint8_t payload[AmfiProtMaxPacketLength - sizeof(lib_AmfiProt_Header_t)]; // Last byte of payload is an 8 bit CRC
};
// static_assert(sizeof(lib_AmfiProt_Frame) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Frame larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_FirmwareStart
{
	uint8_t payloadID;
	uint8_t processorID; // 0: STM32 processor, 1: RF processor
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_FirmwareStart) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_FirmwareStart larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_FirmwareEnd
{
	uint8_t payloadID;
	uint8_t processorID; // 0: STM32 processor, 1: RF processor
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_FirmwareEnd) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_FirmwareEnd larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_FirmwareBinary
{
	uint8_t payloadID;
	uint8_t processorID; // 0: STM32 processor, 1: RF processor
	uint8_t data[AmfiProtMaxPayloadLength - 2 * sizeof(uint8_t)];
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_FirmwareBinary) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_FirwmareBinary larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_DeviceID
{
	uint8_t payloadID;
	uint8_t TxID;
	uint32_t UUID[3];
	uint32_t crc;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_DeviceID) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_DeviceID larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_FirmwareVersion
{
	uint8_t payloadID;
	uint32_t major;
	uint32_t minor;
	uint32_t patch;
	uint32_t build;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_FirmwareVersion) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_FirmwareVersion larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_FirmwareVersionPerIDRequest
{
	uint8_t payloadID;
	uint8_t processorID; // 0: STM32 processor, 1: RF processor
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_FirmwareVersionPerIDRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_FirmwareVersionPerIDRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_FirmwareVersionPerID
{
	uint8_t payloadID;
	uint32_t major;
	uint32_t minor;
	uint32_t patch;
	uint32_t build;
	uint8_t processorID; // 0: STM32 processor, 1: RF processor
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_FirmwareVersionPerID) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_FirmwareVersionPerID larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_DeviceName
{
	uint8_t payloadID;
	char name[AmfiProtMaxPayloadLength - sizeof(uint8_t)];
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_DeviceName) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_DeviceName larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ResetInfoRequest
{
	uint8_t payloadID;
	uint8_t field;              // lib_AmfiProt_ResetInfoField_t
	uint8_t chunkIndex;         // String fields only; ignored for the summary, but always present so the request is fixed size
	uint8_t recordIndex;        // 0 = newest record, higher indices progressively older
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ResetInfoRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ResetInfoRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ResetInfoSummary
{
	uint8_t payloadID;
	uint8_t field;              // Always lib_AmfiProt_ResetInfoField_Summary, so the host can demux the two reply shapes sharing one payload ID
	uint8_t resetReason;        // Decoded from this record's own reset status
	uint8_t recordType;         // lib_AmfiProt_ResetInfoRecord_t
	uint32_t cfsr;              // Hard fault only
	uint32_t xFAR;              // Hard fault only; MMFAR or BFAR, CFSR's MMARVALID/BFARVALID says which
	uint32_t pc;
	uint32_t lr;                // Hard fault only
	uint32_t psr;               // Hard fault only
	uint32_t assertLine;        // Assert only
	uint8_t chunkCount[lib_AmfiProt_ResetInfoField_Last - 1];   // File, func, expr - 0 when that string is unavailable
	uint8_t recordIndex;        // Echoed from the request
	uint8_t recordCount;        // Records available to index, saturating at 255
	uint32_t fw_mmp;            // Build that wrote this record: major << 16 | minor << 8 | patch
	uint32_t fw_build;          // With fw_mmp, names the .elf that resolves pc and stringAddr
	uint32_t stringAddr[lib_AmfiProt_ResetInfoField_Last - 1];  // file, func, expr pointer values; assert only, 0 otherwise
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ResetInfoSummary) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ResetInfoSummary larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ResetInfoChunk
{
	uint8_t payloadID;
	uint8_t field;
	uint8_t chunkIndex;
	uint8_t chunkCount;         // Repeated so a host that lost the summary can still reassemble; carries no recordIndex - the reply's packet number mirrors the request's
	char text[AmfiProtMaxPayloadLength - 4*sizeof(uint8_t)];    // Not nul terminated - the frame length carries the count, so a split string concatenates cleanly
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ResetInfoChunk) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ResetInfoChunk larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigNameRequest
{
	uint8_t payloadID;
	uint16_t index;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigNameRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigNameRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigName
{
	uint8_t payloadID;
	uint16_t index;
	char name[AmfiProtMaxPayloadLength - sizeof(uint8_t) - sizeof(uint16_t)];
}
__packed;
// static_assert(sizeof(struct lib_AmfiProt_ConfigName) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigName larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigNameRequestUID
{
	uint8_t payloadID;
	uint8_t category; // 0xFF for all
	uint16_t index;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigNameRequestUID) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigNameRequestUID larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigNameUID_common
{
	uint8_t category;
	uint32_t uid; // Use the current value from Unix Time Stamp - Epoch Converter  when a new parameter is added
	char name[AmfiProtMaxPayloadLength - 2 * sizeof(uint8_t) - sizeof(uint16_t) - sizeof(uint32_t) - sizeof(uint8_t)];
}
__packed;

__PACKED_STRUCT lib_AmfiProt_ConfigNameUID_protocol
{
	uint8_t payloadID;
	uint16_t index;
	lib_AmfiProt_ConfigNameUID_common_t config_names;
}
__packed;
// static_assert(sizeof(struct lib_AmfiProt_ConfigNameUID_protocol) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigNameUID larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigNameUID_settings
{
	lib_AmfiProt_ConfigNameUID_common_t config_names;
	uint32_t bitmask_Dep;
}
__packed;

__PACKED_STRUCT lib_AmfiProt_ConfigValueCountRequest
{
	uint8_t payloadID;
	uint8_t category; // 0xFF for all
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigValueCountRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigValueCountRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigValueCount
{
	uint8_t payloadID;
	uint8_t category; // 0xFF for all
	uint16_t count;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigValueCount) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigValueCount larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigValueRequest
{
	uint8_t payloadID;
	uint16_t index;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigValueRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigValueRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigValue
{
	uint8_t payloadID;
	uint16_t index;
	lib_Generic_Parameter_Value_t value;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigValue) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigValue larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigValueUIDRequest
{
	uint8_t payloadID;
	uint32_t uid;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigValueUIDRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigValueUIDRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigValueUID
{
	uint8_t payloadID;
	uint32_t uid;
	lib_Generic_Parameter_Value_t value;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigValueUID) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigValueUID larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigCategoryRequest
{
	uint8_t payloadID;
	uint8_t categoryIndex;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigCategoryRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigCategoryRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigCategory
{
	uint8_t payloadID;
	uint8_t categoryIndex;
	char name[AmfiProtMaxPayloadLength - sizeof(uint8_t) - sizeof(uint8_t)];
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigCategory) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigCategory larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ConfigCategoryCount
{
	uint8_t payloadID;
	uint8_t categoryCount;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ConfigCategoryCount) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ConfigCategoryCount larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_LoadDefault
{
	uint8_t payloadID;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_LoadDefault) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_LoadDefault larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_SaveAsDefault
{
	uint8_t payloadID;
	uint32_t UUID[3];
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_SaveAsDefault) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_SaveAsDefault larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ProcedureSpecRequest
{
	uint8_t payloadID;
	uint16_t index; // If index is 0xFFFF, use uid, else ignore uid
	uint32_t uid;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ProcedureSpecRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ProcedureSpecRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ProcedureSpec
{
	uint8_t payloadID;
	uint16_t index;
	uint32_t uid;
	uint8_t return_value_type;
	uint8_t parameter1_type;
	uint8_t parameter2_type;
	uint8_t parameter3_type;
	uint8_t parameter4_type;
	uint8_t parameter5_type;
	char procedure_name[AmfiProtMaxPayloadLength - sizeof(uint8_t) - sizeof(uint16_t) - sizeof(uint32_t) - 6 * sizeof(uint8_t)];
}
__packed;
// static_assert(sizeof(struct lib_AmfiProt_ProcedureSpec) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ProcedureSpec larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ProcedureRequest
{
	uint8_t payloadID;
	uint32_t uid;
	lib_Generic_Parameter_Value_t parameter1;
	lib_Generic_Parameter_Value_t parameter2;
	lib_Generic_Parameter_Value_t parameter3;
	lib_Generic_Parameter_Value_t parameter4;
	lib_Generic_Parameter_Value_t parameter5;
}
__packed;
// static_assert(sizeof(struct lib_AmfiProt_ProcedureRequest) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ProcedureRequest larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_ProcedureReply
{
	uint8_t payloadID;
	uint32_t uid;
	//    uint8_t status;
	lib_Generic_Parameter_Value_t return_value;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_ProcedureReply) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_ProcedureReply larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_DebugOutput
{
	uint8_t payloadID;
	char debugString[AmfiProtMaxPayloadLength - sizeof(uint8_t)];
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_DebugOutput) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_DebugOutput larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_RebootDevice
{
	uint8_t payloadID;
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_RebootDevice) <= AmfiProtMaxPacketLength, "lib_AmfiProt_RebootDevice larger than max payload size");

__PACKED_STRUCT lib_AmfiProt_TunnelData
{
	uint8_t payloadID;
	uint8_t endPoint;
	uint8_t data[AmfiProtMaxPayloadLength - 2 * sizeof(uint8_t)];
}
__packed;
static_assert(sizeof(struct lib_AmfiProt_TunnelData) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_TunnelData larger than max payload size");

#ifdef LIB_AMFIPROT_PACKED_PUSHED
#pragma pack(pop)
#undef LIB_AMFIPROT_PACKED_PUSHED
#endif

#endif // LIB_AMFIPROT_H_H
