//-----------------------------------------------------------------------------
//                              AMFITECH APS
//                          ALL RIGHTS RESERVED
//
// $URL: $
// $Rev: $
// $Date: $
// $Author: $
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
/// \cond
#include <cstring>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h> // For memcpy
/// \endcond

#include "lib_AmfiProt.hpp"
#include "lib_log.h"
#include "lib_CRC.hpp"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
// #define USE_AMFIPROT_LEGACY_TYPES
//-----------------------------------------------------------------------------
// Type declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variables and constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
lib_AmfiProt::lib_AmfiProt()
	: deviceID(0)
{
}

lib_AmfiProt::~lib_AmfiProt()
{
}

bool lib_AmfiProt::lib_AmfiProt_UpdateCRC(lib_AmfiProt_Frame_t *frame)
{
	assert(frame);

	bool retVal = false;

	if (frame->header.length <= sizeof(frame->payload))
	{
		CRC8 crc;
		crc.crc(&(frame->header), sizeof(frame->header) - 1);
		frame->header.headCRC = crc.getCRC();

		if (frame->header.length > 0)
		{
			crc.reset();
			crc.crc(frame->payload, frame->header.length);
			frame->payload[frame->header.length] = crc.getCRC();
		}
		retVal = true;
	}

	return retVal;
}

bool lib_AmfiProt::lib_AmfiProt_EncodeFrame(lib_AmfiProt_Frame_t *frame, void const *pPayload, uint8_t length, uint8_t payloadType, uint8_t packetNumber, uint8_t destination, lib_AmfiProt_packetType_t packetType)
{
	assert(frame);

	bool retVal = false;

	if (length <= sizeof(frame->payload))
	{
		frame->header.length = length;
		frame->header.source = this->deviceID;
		frame->header.destination = destination;
		frame->header.packetNumber = packetNumber;
		frame->header.payloadType = payloadType;
		frame->header.packetType = (uint8_t)packetType;

		if (frame->header.length > 0)
		{
			memcpy(frame->payload, pPayload, length);
		}

		if (lib_AmfiProt_UpdateCRC(frame))
		{
			retVal = true;
		}
	}

	return retVal;
}

bool lib_AmfiProt::lib_AmfiProt_EncodeAck(lib_AmfiProt_Frame_t *incomingFrame, lib_AmfiProt_Frame_t *outgoingFrame)
{
	assert(incomingFrame);
	assert(outgoingFrame);

	bool retVal = true;

	outgoingFrame->header.length = 0;
	outgoingFrame->header.source = this->deviceID;
	outgoingFrame->header.destination = incomingFrame->header.source;
	outgoingFrame->header.payloadType = incomingFrame->header.payloadType;
	outgoingFrame->header.packetNumber = incomingFrame->header.packetNumber;
	outgoingFrame->header.packetType = lib_AmfiProt_packetType_Ack;

	CRC8 crc;
	crc.crc(&(outgoingFrame->header), sizeof(outgoingFrame->header) - 1);
	outgoingFrame->header.headCRC = crc.getCRC();

	return retVal;
}

bool lib_AmfiProt::lib_AmfiProt_DeserializeFrame(lib_AmfiProt_Frame_t *frame, void const *pData, uint8_t length)
{
	assert(frame);
	assert(pData);

	bool retVal = false;

	if (length >= sizeof(frame->header)) // OK, we have received at least a complete header
	{
		if (length > sizeof(*frame))
		{
			memcpy(frame, pData, sizeof(*frame));
		}
		else
		{
			memcpy(frame, pData, length);
		}
		CRC8 crc;
		crc.crc(&(frame->header), sizeof(frame->header) - 1);
		if (crc.getCRC() == frame->header.headCRC)
		{
			if (frame->header.length <= (length - sizeof(frame->header)))
			{
				crc.reset();
				crc.crc(frame->payload, frame->header.length);
				if ((frame->header.length == 0) ||
					(crc.getCRC() == frame->payload[frame->header.length]))
				{
					retVal = true;
				}
				else
				{
					LOG_W("payloadCRC failed, was: 0x%02X, should have been: 0x%02X", crc.getCRC(), frame->payload[frame->header.length]);
				}
			}
		}
		else
		{
			LOG_W("headCRC failed, was: 0x%02X, should have been: 0x%02X", crc.getCRC(), frame->header.headCRC);
		}
	}

	if (!retVal)
	{
		LOG_W("Deserialization failed!");
		memset(frame, 0, sizeof(*frame)); // Clear frame if it was not decoded correctly
	}

	return retVal;
}

uint8_t lib_AmfiProt::lib_AmfiProt_FrameSize(lib_AmfiProt_Frame_t const *frame)
{
	assert(frame);

	uint8_t frameSize;

	if (frame->header.length == 0) // Don't include payload CRC for empty payloads
	{
		frameSize = sizeof(frame->header);
	}
	else
	{
		frameSize = sizeof(frame->header) + frame->header.length + 1;
	}

	return frameSize;
}

void lib_AmfiProt::lib_AmfiProt_SetDeviceID(uint8_t _deviceID)
{
	this->deviceID = _deviceID;
}

bool lib_AmfiProt::lib_AmfiProt_Init(lib_AmfiProt_Handle_t *handle, uint8_t _deviceID)
{
	assert(handle);

	lib_AmfiProt_SetDeviceID(_deviceID);

	return true;
}

void lib_AmfiProt::lib_AmfiProt_ProcessFrame(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	uint8_t controlBits = (frame->header.packetType & lib_AmfiProt_packetType_Mask);
	if (controlBits == lib_AmfiProt_packetType_Ack)
	{
		this->libAmfiProt_handle_Ack(handle, frame, routing_handle);
	}
	else
	{
		if (frame->header.payloadType == libAmfiProt_PayloadType_Common)
		{
			if (frame->header.length > 0) // There are no valid frames with payloadType = libAmfiProt_PayloadType_Common, and length = 0
			{
				switch (frame->payload[0])
				{
					case lib_AmfiProt_PayloadID_RequestDeviceID:
					{
						if (frame->header.length == 1)
						{
							this->libAmfiProt_handle_RequestDeviceID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyDeviceID:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_DeviceID_t))
						{
							this->libAmfiProt_handle_RespondDeviceID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_SetTxID:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_DeviceID_t))
						{
							lib_AmfiProt_DeviceID_t rxPayload;
							memcpy(&rxPayload, frame->payload, frame->header.length);

							this->libAmfiProt_handle_SetTxID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestFirmwareVersion:
					{
						if (frame->header.length == 1)
						{
							this->libAmfiProt_handle_RequestFirmwareVersion(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyFirmwareVersion:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_FirmwareVersion_t))
						{
							this->libAmfiProt_handle_FirmwareVersion(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_FirmwareStart:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_FirmwareStart_t))
						{
							this->libAmfiProt_handle_FirmwareStart(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_FirmwareData:
					{
						if (frame->header.length <= sizeof(lib_AmfiProt_FirmwareBinary_t))
						{
							this->libAmfiProt_handle_FirmwareData(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_FirmwareEnd:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_FirmwareEnd_t))
						{
							this->libAmfiProt_handle_FirmwareEnd(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestDeviceName:
					{
						if (frame->header.length == 1)
						{
							this->libAmfiProt_handle_RequestDeviceName(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyDeviceName:
					{
						if (frame->header.length <= sizeof(lib_AmfiProt_DeviceName_t))
						{
							this->libAmfiProt_handle_ReplyDeviceName(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestConfigurationValue:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigValueRequest_t))
						{
							this->libAmfiProt_handle_RequestConfigurationValue(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyConfigurationValue:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigValue_t))
						{
							this->libAmfiProt_handle_ReplyConfigurationValue(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_SetConfigurationValue:
					{
						size_t maximum_payload_size = sizeof(lib_AmfiProt_ConfigValue_t);
						size_t minimum_payload_size = sizeof(lib_AmfiProt_ConfigValue_t) - sizeof(lib_Generic_Parameter_Value_t) + sizeof_member(lib_Generic_Parameter_Value_t, type);
						if (frame->header.length >= minimum_payload_size && frame->header.length <= maximum_payload_size)
						{
							this->libAmfiProt_handle_SetConfigurationValue(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestConfigurationName:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigNameRequest_t))
						{
							this->libAmfiProt_handle_RequestConfigurationName(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyConfigurationName:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigName_t))
						{
							this->libAmfiProt_handle_ReplyConfigurationName(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_LoadDefault:
					{
						if (frame->header.length == 1)
						{
							this->libAmfiProt_handle_LoadDefault(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_SaveAsDefault:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_SaveAsDefault_t))
						{
							this->libAmfiProt_handle_SaveAsDefault(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestConfigurationNameAndUID:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigNameRequestUID_t))
						{
							this->libAmfiProt_handle_RequestConfigurationNameAndUID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyConfigurationNameAndUID:
					{
						if (frame->header.length <= sizeof(lib_AmfiProt_ConfigNameUID_protocol_t))
						{
							this->libAmfiProt_handle_ConfigurationNameAndUID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestConfigurationValueUID:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigValueRequest_t))
						{
							this->libAmfiProt_handle_RequestConfigurationValueUID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyConfigurationValueUID:
					{
						lib_AmfiProt_ConfigValueUID config;
						memcpy(&config, frame->payload, frame->header.length);
						uint8_t size = sizeof(config) - sizeof(config.value) + lib_Generic_Parameter_SizeWithType(config.value);
						if (frame->header.length == size)
						{
							this->libAmfiProt_handle_ConfigurationValueUID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_SetConfigurationValueUID:
					{
						size_t maximum_payload_size = sizeof(lib_AmfiProt_ConfigValueUID_t);
						size_t minimum_payload_size = sizeof(lib_AmfiProt_ConfigValueUID_t) - sizeof(lib_Generic_Parameter_Value_t) + sizeof_member(lib_Generic_Parameter_Value_t, type);
						if (frame->header.length >= minimum_payload_size && frame->header.length <= maximum_payload_size)
						{
							this->libAmfiProt_handle_SetConfigurationValueUID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestConfigurationCategory:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigCategoryRequest_t))
						{
							this->libAmfiProt_handle_RequestConfigurationCategory(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyConfigurationCategory:
					{
						if (frame->header.length <= sizeof(lib_AmfiProt_ConfigCategory_t))
						{
							this->libAmfiProt_handle_ConfigurationCategory(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestConfigurationValueCount:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigValueCountRequest_t))
						{
							this->libAmfiProt_handle_RequestConfigurationValueCount(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyConfigurationValueCount:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigValueCount_t))
						{
							this->libAmfiProt_handle_ConfigurationValueCount(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestCategoryCount:
					{
						if (frame->header.length == 1)
						{
							this->libAmfiProt_handle_RequestCategoryCount(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyCategoryCount:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ConfigCategoryCount_t))
						{
							this->libAmfiProt_handle_CategoryCount(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestResetInfo:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ResetInfoRequest_t))
						{
							this->libAmfiProt_handle_RequestResetInfo(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyResetInfo:
					{
						if (frame->header.length <= sizeof(lib_AmfiProt_ResetInfoChunk_t))
						{
							this->libAmfiProt_handle_ReplyResetInfo(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_DebugOutput:
					{
						if (frame->header.length <= sizeof(lib_AmfiProt_DebugOutput_t))
						{
							this->libAmfiProt_handle_DebugOutput(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_Reboot:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_RebootDevice_t))
						{
							this->libAmfiProt_handle_Reboot(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestEventLog:
					{
						// TODO: Not implemented yet
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyEventLog:
					{
						// Not handled by default
					}
					break;
					case lib_AmfiProt_PayloadID_ResetParameter:
					{
						if (frame->header.length == 2)
						{
							this->libAmfiProt_handle_ResetParameter(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestFirmwareVersionPerID:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_FirmwareVersionPerIDRequest_t))
						{
							this->libAmfiProt_handle_RequestFirmwareVersionPerID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyFirmwareVersionPerID:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_FirmwareVersionPerID_t))
						{
							this->libAmfiProt_handle_ReplyFirmwareVersionPerID(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestProcedureSpec:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ProcedureSpecRequest_t))
						{
							lib_AmfiProt_ProcedureSpecRequest_t rxPayload;
							memcpy(&rxPayload, frame->payload, frame->header.length);
							this->libAmfiProt_handle_RequestProcedureSpec(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyProcedureSpec:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ProcedureSpec_t))
						{
							this->libAmfiProt_handle_ReplyProcedureSpec(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_RequestProcedureCall:
					{
						size_t maximum_payload_size = sizeof(lib_AmfiProt_ProcedureRequest_t);
						size_t minimum_payload_size = sizeof(lib_AmfiProt_ProcedureRequest_t) - 5 * sizeof(lib_Generic_Parameter_Value_t);
						if (frame->header.length >= minimum_payload_size && frame->header.length <= maximum_payload_size)
						{
							lib_AmfiProt_ProcedureRequest_t rxPayload;
							rxPayload.parameter1.type = lib_Generic_Parameter_Type_void;
							rxPayload.parameter2.type = lib_Generic_Parameter_Type_void;
							rxPayload.parameter3.type = lib_Generic_Parameter_Type_void;
							rxPayload.parameter4.type = lib_Generic_Parameter_Type_void;
							rxPayload.parameter5.type = lib_Generic_Parameter_Type_void;

							memcpy(&rxPayload, frame->payload, frame->header.length);

							this->libAmfiProt_handle_RequestProcedureCall(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;
					case lib_AmfiProt_PayloadID_ReplyProcedureCall:
					{
						if (frame->header.length == sizeof(lib_AmfiProt_ProcedureReply_t))
						{
							lib_AmfiProt_ProcedureReply_t rxPayload;
							memcpy(&rxPayload, frame->payload, frame->header.length);

							this->libAmfiProt_handle_ReplyProcedureCall(handle, frame, routing_handle);
						}
						else
						{
							this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
						}
					}
					break;

					default:
					{
						this->libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
					}
					break;
				}
			}
		}
		else if (frame->header.payloadType >= libAmfiProt_PayloadType_Success)
		{
			switch (frame->header.payloadType)
			{
				case libAmfiProt_PayloadType_Success:
				{
					this->libAmfiProt_handle_ReplySuccess(handle, frame, routing_handle);
				}
				break;
				case libAmfiProt_PayloadType_NotImplemented:
				{
					this->libAmfiProt_handle_ReplyNotImplemented(handle, frame, routing_handle);
				}
				break;
				case libAmfiProt_PayloadType_Failure:
				{
					this->libAmfiProt_handle_ReplyFailure(handle, frame, routing_handle);
				}
				break;
				case libAmfiProt_PayloadType_InvalidRequest:
				{
					this->libAmfiProt_handle_ReplyInvalidRequest(handle, frame, routing_handle);
				}
				break;
			}
		}
		else
		{
			this->libAmfiProt_handle_AlternativeProcessing(handle, frame, routing_handle);
		}
	}
}