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
#include "Amfitrack_Sensor.h"

#include <cstring>
//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Section: Typedef
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Section: Macro
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Section: Variables
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Section: Function prototypes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Section: Functions
//-----------------------------------------------------------------------------

AMFITRACK_Sensor::AMFITRACK_Sensor()
	: AMFITRACK_Sensor(0)
{
}

AMFITRACK_Sensor::AMFITRACK_Sensor(uint8_t id)
{
	reset();
	deviceId = id;
}

void AMFITRACK_Sensor::reset()
{
	deviceId = 0;
	std::memset(name, 0, sizeof(name));
	std::memset(uuid, 0, sizeof(uuid));
	std::memset(&FW_Version, 0, sizeof(FW_Version));
	std::memset(&RF_Version, 0, sizeof(RF_Version));
	std::memset(&HW_Version, 0, sizeof(HW_Version));
	config = DeviceConfig_t{};
	resetInfo = ResetInfoLog_t{};
	active = false;
	hub_ID = 0;
	calcId = 0;
	_dev_handle = nullptr;
	std::memset(&status, 0, sizeof(status));
	std::memset(&external_input, 0, sizeof(external_input));
	std::memset(&pose, 0, sizeof(pose));
	std::memset(&imu, 0, sizeof(imu));
	std::memset(&rawBfield, 0, sizeof(rawBfield));
	std::memset(&normBfield, 0, sizeof(normBfield));
	std::memset(&rawWithPhaseBfield, 0, sizeof(rawWithPhaseBfield));
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
	timestamp = std::chrono::steady_clock::time_point{};
#endif
	std::memset(&sensorMeasurement, 0, sizeof(sensorMeasurement));
	lastTimeSeenMs = 0;
}
