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
#include "lib_AmfiProt_API.hpp"
#include "lib_log.h"

#ifdef USE_THREAD_BASED
#include <iostream>
#include <thread>
#endif

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Type declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variables and constants
//-----------------------------------------------------------------------------

// Time in ms until transmit is counted as timed out
static constexpr std::chrono::milliseconds kRetransmitTimeout{1000};

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
AmfiProt_API::AmfiProt_API()
{
}

AmfiProt_API::~AmfiProt_API()
{
}

void AmfiProt_API::isRequestAckSet(bool removeFromQueue)
{
	// To check if its requesting an ack (Can it be done a better way?)
	lib_AmfiProt_Frame_t amfiFrame;

	if (!outgoingBulk_FiFo.peek(amfiFrame))
	{
		return;
	}

	if (removeFromQueue)
	{
		outgoingBulk_FiFo.pop(amfiFrame);
		this->isTransmitting = false;
		this->_retransmitCount = 0;
		return;
	}

	uint8_t controlBits = amfiFrame.header.packetType &
						  lib_AmfiProt_packetType_Mask;

	if (controlBits)
	{
		this->isTransmitting = true;
		_retransmitTimer = std::chrono::steady_clock::now();
	}
	else
	{
		// Set is transmitting to false since theres no ack later here
		outgoingBulk_FiFo.pop(amfiFrame);
		this->isTransmitting = false;
		this->_retransmitCount = 0;
	}
}

void AmfiProt_API::process_incoming_queue(void)
{

	while (!incomingBulk_FiFo.isEmpty())
	{
		lib_AmfiProt_Frame_t frame;
		incomingBulk_FiFo.pop(frame);
		this->lib_AmfiProt_ProcessFrame(NULL, &frame, NULL);
	}
}

void AmfiProt_API::clear_isTransmitting(lib_AmfiProt_Frame_t *frame)
{
	lib_AmfiProt_Frame_t *inFlight = outgoingBulk_FiFo.peek();

	if (inFlight == nullptr)
	{
		return;
	}

	// The ack must come from the device this frame was addressed to.
	if (inFlight->header.packetNumber != frame->header.packetNumber ||
		inFlight->header.destination != frame->header.source)
	{
		return;
	}

	lib_AmfiProt_Frame_t acked;
	outgoingBulk_FiFo.pop(acked);
	this->_retransmitCount = 0;
	this->isTransmitting = false;
}

bool AmfiProt_API::queue_frame(void const *payload, uint8_t length, uint8_t payloadType, lib_AmfiProt_packetType_t packetType, uint8_t destination)
{
	static uint8_t packageNumber = 0;

	bool isOk = false;
	lib_AmfiProt_Frame_t amfiFrame;

	packetNumber[destination] = packageNumber;
	if (this->lib_AmfiProt_EncodeFrame(&amfiFrame, payload, length, payloadType, packetNumber[destination], destination, packetType))
	{
		if (!outgoingBulk_FiFo.isFull())
		{
			outgoingBulk_FiFo.put(amfiFrame);
			isOk = true;
			packageNumber++;
		}
	}

	return isOk;
}

bool AmfiProt_API::deserialize_frame(void const *pData, uint8_t length)
{
	bool isOk = false;
	lib_AmfiProt_Frame_t frame;
	if (lib_AmfiProt_DeserializeFrame(&frame, pData, length))
	{
		if (!incomingBulk_FiFo.isFull())
		{
			incomingBulk_FiFo.put(frame);
			isOk = true;
		}
		else
		{
			LOG_W("Queue full");
		}
	}
	return isOk;
}

bool AmfiProt_API::isDataReadyForTransmit(size_t *QueueDataLength, uint8_t *TxID, void **TransmitData)
{
	if (isTransmitting || outgoingBulk_FiFo.isEmpty())
	{
		return false;
	}

	lib_AmfiProt_Frame_t *frame = outgoingBulk_FiFo.peek();

	if (frame == nullptr)
	{
		return false;
	}
	*QueueDataLength = frame->header.length + sizeof(lib_AmfiProt_Header) + 1;
	*TxID = frame->header.destination;
	*TransmitData = frame;

	return true;
}

void AmfiProt_API::set_transmit_ongoing_and_check_respons_request(bool removeFromQueue)
{
	isTransmitting = true;
	isRequestAckSet(removeFromQueue);
}

void AmfiProt_API::amfiprot_run(void)
{
	this->process_incoming_queue();

	if (this->isTransmitting &&
		(std::chrono::steady_clock::now() - _retransmitTimer) >= kRetransmitTimeout)
	{
		this->_retransmitCount++;
		if (this->_retransmitCount >= 3)
		{
			this->_retransmitCount = 0;
			lib_AmfiProt_Frame_t frame;
			outgoingBulk_FiFo.pop(frame);
		}
		this->isTransmitting = false;
	}
}

void AmfiProt_API::libAmfiProt_handle_Ack(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;

	LOG_D("TxUID: %u | Ack", frame->header.source);
	this->clear_isTransmitting(frame);
}

void AmfiProt_API::libAmfiProt_handle_ReplySuccess(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	LOG_D("TxUID: %u | Success reply", frame->header.source);
}

void AmfiProt_API::libAmfiProt_handle_ReplyFailure(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	LOG_D("TxUID: %u | Failure reply", frame->header.source);
}

void AmfiProt_API::libAmfiProt_ReplyInvalid(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	LOG_D("TxUID: %u | Invalid reply: %u", frame->header.source, frame->header.payloadType);
}

void AmfiProt_API::libAmfiProt_handle_ReplyNotImplemented(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	LOG_D("TxUID: %u | Not implemented reply: %u", frame->header.source, frame->header.payloadType);
}

void AmfiProt_API::libAmfiProt_handle_ReplyInvalidRequest(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	LOG_D("TxUID: %u | Invalid request: %u", frame->header.source, frame->header.payloadType);
}

void AmfiProt_API::libAmfiProt_handle_AlternativeProcessing(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	this->lib_AmfiProt_Amfitrack_processFrame(handle, frame, routing_handle);
}
