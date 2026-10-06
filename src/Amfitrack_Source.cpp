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
#include "Amfitrack_Source.h"

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
AMFITRACK_Source::AMFITRACK_Source()
	: AMFITRACK_Source(0)
{
}

AMFITRACK_Source::AMFITRACK_Source(uint8_t id)
{
	reset();
	deviceId = id;
}

void AMFITRACK_Source::reset()
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
	_dev_handle = nullptr;
	std::memset(&status, 0, sizeof(status));
	std::memset(&imu, 0, sizeof(imu));
	std::memset(&current, 0, sizeof(current));
	std::memset(&frequency, 0, sizeof(frequency));
	std::memset(&voltage, 0, sizeof(voltage));
	std::memset(&calibration, 0, sizeof(calibration));
	lastTimeSeenMs = 0;
}
