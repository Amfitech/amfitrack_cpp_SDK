//-----------------------------------------------------------------------------
//
//                              AMFITECH APS
//
//                          ALL RIGHTS RESERVED
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Includes
//-----------------------------------------------------------------------------
#include "Amfitrack_task.h"

#include "lib_AmfiProt_API.hpp"
#include "Amfitrack_Devices.h"
#include "lib_time.h"

#include <cstddef>
#include <mutex>
//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------
#define DISCONNECT_TIMEOUT 5000
//-----------------------------------------------------------------------------
// Section: Typedef
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Macro
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Section: Variables
//-----------------------------------------------------------------------------
static AmfiProt_API *amfiprot_api = nullptr;
amfitrack_task::missingInfo_t amfitrack_task::missingInfo = amfitrack_task::missingInfo_t::missingInfo_FW;

static constexpr uint32_t kKeepAlivePingIntervalMs = 2000;
static uint32_t lastKeepAlivePingTimeMs = 0;
static std::mutex keepAlivePingMutex;

static constexpr uint32_t kGetMissingInfoIntervalMs = 2000;
static uint32_t lastGetMissingInfoTimeMs = 0;
static std::mutex getMissingInfoMutex;
//-----------------------------------------------------------------------------
// Section: Function prototypes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Section: Functions
//-----------------------------------------------------------------------------
void amfitrack_task::getVersion(uint8_t deviceID, uint8_t _version)
{
	lib_AmfiProt_FirmwareVersionPerIDRequest payload;
	payload.payloadID = lib_AmfiProt_PayloadID_RequestFirmwareVersionPerID;
	payload.processorID = _version;
	amfiprot_api->queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, deviceID);
}

void amfitrack_task::getName(uint8_t deviceID)
{
	uint8_t payload = lib_AmfiProt_PayloadID_RequestDeviceName;
	amfiprot_api->queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, deviceID);
}

void amfitrack_task::getMissingInfo()
{
	const uint32_t now = lib_time::get_time_ms();

	const std::lock_guard<std::mutex> lock(getMissingInfoMutex);

	if (lastGetMissingInfoTimeMs == 0 ||
		(now - lastGetMissingInfoTimeMs) >= kGetMissingInfoIntervalMs)
	{
		// Sensors and sources expose the same version/name fields, so both kinds
		// are queried for whatever info is currently missing.
		for (uint8_t i = 0; i < AMFITRACK_DEVICE_COUNT; i++)
		{
			AMFITRACK_Sensor sensor;
			AMFITRACK_Devices::getInstance().get_sensor_by_id(i, &sensor);
			if (sensor.active)
			{
				requestMissingInfo(i, sensor.FW_Version, sensor.RF_Version,
								   sensor.HW_Version, sensor.name);
			}

			AMFITRACK_Source source;
			AMFITRACK_Devices::getInstance().get_source_by_id(i, &source);
			if (source.active)
			{
				requestMissingInfo(i, source.FW_Version, source.RF_Version,
								   source.HW_Version, source.name);
			}
		}
		switch (missingInfo)
		{
			case amfitrack_task::missingInfo_t::missingInfo_FW:
				missingInfo = amfitrack_task::missingInfo_t::missingInfo_RF;
				break;
			case amfitrack_task::missingInfo_t::missingInfo_RF:
				missingInfo = amfitrack_task::missingInfo_t::missingInfo_HW;
				break;
			case amfitrack_task::missingInfo_t::missingInfo_HW:
				missingInfo = amfitrack_task::missingInfo_t::missingInfo_Name;
				break;
			case amfitrack_task::missingInfo_t::missingInfo_Name:
				missingInfo = amfitrack_task::missingInfo_t::missingInfo_FW;
				break;
		}
		lastGetMissingInfoTimeMs = now;
	}
}

void amfitrack_task::requestMissingInfo(uint8_t deviceID, const FW_t &fw, const RF_t &rf,
										const HW_t &hw, const char *name)
{
	switch (missingInfo)
	{
		case amfitrack_task::missingInfo_t::missingInfo_FW:
			if (fw.Major == 0)
			{
				getVersion(deviceID, AMFITRACK_FW_VERSION_ID);
			}
			break;
		case amfitrack_task::missingInfo_t::missingInfo_RF:
			if (rf.Major == 0)
			{
				getVersion(deviceID, AMFITRACK_RF_VERSION_ID);
			}
			break;
		case amfitrack_task::missingInfo_t::missingInfo_HW:
			if (hw.Generation == 0)
			{
				getVersion(deviceID, AMFITRACK_HW_VERSION_ID);
			}
			break;
		case amfitrack_task::missingInfo_t::missingInfo_Name:
			if (name[0] == 0x00)
			{
				getName(deviceID);
			}
			break;
	}
}

void amfitrack_task::keepAlivePing()
{
	uint8_t payload = lib_AmfiProt_PayloadID_RequestDeviceID;
	amfiprot_api->queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, lib_AmfiProt_destination_Broadcast);
}

void amfitrack_task::checkDisconnected()
{
	AMFITRACK_Devices &devices = AMFITRACK_Devices::getInstance();
	const uint32_t now = lib_time::get_time_ms();

	for (std::size_t device_index = 0; device_index < AMFITRACK_Devices::device_count(); device_index++)
	{
		const auto device_id = static_cast<uint8_t>(device_index);
		AMFITRACK_Sensor _sensor;
		devices.get_sensor_by_id(device_id, &_sensor);
		if (_sensor.active && ((now - _sensor.lastTimeSeenMs) > DISCONNECT_TIMEOUT))
		{
			devices.set(device_id, AMFITRACK_Devices::deviceType_t::Sensor, false);
		}

		AMFITRACK_Source _source;
		devices.get_source_by_id(device_id, &_source);
		if (_source.active && ((now - _source.lastTimeSeenMs) > DISCONNECT_TIMEOUT))
		{
			devices.set(device_id, AMFITRACK_Devices::deviceType_t::Source, false);
		}
	}
}

void amfitrack_task::init()
{
	const std::lock_guard<std::mutex> lock(keepAlivePingMutex);
	amfiprot_api = &AmfiProt_API::getInstance();
	const uint32_t now = lib_time::get_time_ms();

	lastKeepAlivePingTimeMs = now;
	lastGetMissingInfoTimeMs = now;
}

void amfitrack_task::run()
{
	const uint32_t now = lib_time::get_time_ms();

	{
		const std::lock_guard<std::mutex> lock(keepAlivePingMutex);

		if (amfiprot_api != nullptr &&
			(lastKeepAlivePingTimeMs == 0 ||
			 (now - lastKeepAlivePingTimeMs) >= kKeepAlivePingIntervalMs))
		{
			keepAlivePing();
			lastKeepAlivePingTimeMs = now;
		}
	}

	checkDisconnected();
	getMissingInfo();
}
