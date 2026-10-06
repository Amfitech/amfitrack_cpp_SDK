//-----------------------------------------------------------------------------
//                              AMFITECH APS
//                          ALL RIGHTS RESERVED
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Section: Includes
//-----------------------------------------------------------------------------
#include "Amfitrack_Sensor.h"
#include "Amfitrack_Source.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

#ifdef USE_THREAD_BASED
#include <mutex>
#endif
//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------
#define AMFITRACK_DEVICE_COUNT 255
#define AMFITRACK_BROADCAST_DEVICE_ID 255U

#define AMFITRACK_FW_VERSION_ID 0
#define AMFITRACK_RF_VERSION_ID 1
#define AMFITRACK_HW_VERSION_ID 255
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

class AMFITRACK_Devices
{

  public:
	enum class deviceType_t
	{
		None,
		Sensor,
		Source,
		Both,
	};

	static AMFITRACK_Devices &getInstance();

	static uint32_t device_count();
	static bool is_valid_device_id(uint8_t device_id);

	bool get_sensor_by_id(uint8_t device_id, AMFITRACK_Sensor *sensor);
	bool get_source_by_id(uint8_t device_id, AMFITRACK_Source *source);
	bool get_sensor_by_number(uint8_t device_number, AMFITRACK_Sensor *sensor);
	bool get_source_by_number(uint8_t device_number, AMFITRACK_Source *source);
	bool is_device_active(uint8_t device_id);

	uint8_t get_numer_of_sensors();
	uint8_t get_numer_of_sources();

	bool set(uint8_t device_id, deviceType_t type, bool isActive);
	bool set(uint8_t device_id, deviceType_t type, char const *name, uint8_t length);
	bool set(uint8_t device_id, deviceType_t type, uint32_t UUID1, uint32_t UUID2, uint32_t UUID3);
	bool set(uint8_t device_id, deviceType_t type, FW_t fwVersion);
	bool set(uint8_t device_id, deviceType_t type, RF_t rfVersion);
	bool set(uint8_t device_id, deviceType_t type, HW_t hwVersion);
	bool set(uint8_t device_id, deviceType_t type, uint8_t hubId);
	bool set_hid(uint8_t device_id, deviceType_t type, hid_device *hidHandle);
	bool set(uint8_t device_id, deviceType_t type, DeviceConfig_t const &config);
	bool set(uint8_t device_id, deviceType_t type, ResetInfoLog_t const &resetInfo);

	bool set(uint8_t device_id, deviceType_t type, IMU_t const &imu);

	bool set(uint8_t device_id, Sensor_Status_t const &status);
	bool set(uint8_t device_id, External_input_t const &ext_input);
	bool set(uint8_t device_id, Pose_t const &pose);
	bool set(uint8_t device_id, Raw_B_Field_t const &rawBfield);
	bool set(uint8_t device_id, Normalized_B_Field_t const &normBfield);
	bool set(uint8_t device_id, Raw_with_Phase_B_Field_t const &rawWithPhaseBfield);
	// Deprecated!
	bool set(uint8_t device_id, lib_AmfiProt_Amfitrack_Sensor_Measurement_t const &sensorMeasurement);

	bool set(uint8_t device_id, Source_Status_t const &status);
	bool set(uint8_t device_id, Current_t const &current);
	bool set(uint8_t device_id, Frequency_t const &frequency);
	bool set(uint8_t device_id, Voltage_t const &voltage);
	bool set(uint8_t device_id, Calibration_t const &calibration);

  private:
	AMFITRACK_Devices();
	~AMFITRACK_Devices() = default;

	AMFITRACK_Devices(AMFITRACK_Devices const &) = delete;
	AMFITRACK_Devices &operator=(AMFITRACK_Devices const &) = delete;

	void update_last_seen(uint8_t device_id, deviceType_t type);

	deviceType_t device_id_exist(uint8_t device_id);
	AMFITRACK_Sensor *get_or_create_sensor(uint8_t device_id);
	AMFITRACK_Source *get_or_create_source(uint8_t device_id);

#ifdef USE_THREAD_BASED
	mutable std::mutex _mutex;
#endif

	std::unordered_map<uint8_t, AMFITRACK_Sensor> _sensors;
	std::unordered_map<uint8_t, AMFITRACK_Source> _sources;
};
