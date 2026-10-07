//-----------------------------------------------------------------------------
//                              AMFITECH APS
//                          ALL RIGHTS RESERVED
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Section: Includes
//-----------------------------------------------------------------------------
#include "AmfitrackDeviceTypes.h"
#include "lib_AmfiProt_Amfitrack.hpp"
#include "HID_Monitor.h"

#include <chrono>
#include <cstdint>

#ifdef USE_USB
#include "hidapi.h"
#endif
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
// Section: Class
//-----------------------------------------------------------------------------

class AMFITRACK_Sensor
{
  public:
	AMFITRACK_Sensor();
	explicit AMFITRACK_Sensor(uint8_t id);

	void reset();

  public:
	uint8_t deviceId;
	char name[MAX_NAME_LENGTH];
	uint32_t uuid[3];
	FW_t FW_Version;
	RF_t RF_Version;
	HW_t HW_Version;
	DeviceConfig_t config;

	bool active;
	uint8_t hub_ID;
	uint16_t calcId;

	hid_device *_dev_handle;

	Sensor_Status_t status;
	External_input_t external_input;
	Pose_t pose;
	IMU_t imu;
	Magneto_t magneto;
	Raw_B_Field_t rawBfield;
	Normalized_B_Field_t normBfield;
	Raw_with_Phase_B_Field_t rawWithPhaseBfield;

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
	std::chrono::steady_clock::time_point timestamp;
#endif

	// Deprecated!
	lib_AmfiProt_Amfitrack_Sensor_Measurement_t sensorMeasurement;

	// Last time a message is received from the device
	uint32_t lastTimeSeenMs;
};
