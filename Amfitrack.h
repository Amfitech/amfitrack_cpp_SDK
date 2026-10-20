//-----------------------------------------------------------------------------
//
//                              AMFITECH APS
//
//                          ALL RIGHTS RESERVED
//
//-----------------------------------------------------------------------------

#pragma once
#ifdef __cplusplus

//-----------------------------------------------------------------------------
// Section: Includes
//-----------------------------------------------------------------------------
#include "Amfitrack_Sensor.h"
#include "Amfitrack_Source.h"
#include "Amfitrack_config.h"
#include "Amfitrack_resetinfo.h"

#include "lib_AmfiProt_API.hpp"

#include <cstdint>
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
class AMFITRACK
{
  public:
	static AMFITRACK &getInstance();

	void init();
	void start_task();
	void stop_task();
	void run();

	bool get_sensor(uint8_t device_id, AMFITRACK_Sensor *sensor) const;
	bool get_source(uint8_t device_id, AMFITRACK_Source *source) const;
	bool get_sensor_by_number(uint8_t device_number, AMFITRACK_Sensor *sensor) const;
	bool get_source_by_number(uint8_t device_number, AMFITRACK_Source *source) const;
	uint8_t get_sensors_active() const;
	uint8_t get_sources_active() const;

	bool setConfiguration(uint8_t DeviceID, uint32_t UID, lib_Generic_Parameter_Value_t parameter);
	bool getConfiguration(uint8_t DeviceID, bool force_all = false);
	ConfigDiscoveryState_t getConfigurationState(uint8_t DeviceID) const;

	// `RecordCount` is how many of the newest fault records to walk: 0 or 1 is the
	// newest only, 3 the newest three. Clamped to what the device reports holding.
	bool requestResetInfo(uint8_t DeviceID, uint8_t RecordCount = 0U);
	ResetInfoState_t getResetInfoState(uint8_t DeviceID) const;
	bool getResetInfoLog(uint8_t DeviceID, ResetInfoLog_t *resetInfoLog) const;
	bool getResetInfo(uint8_t DeviceID, uint8_t RecordIndex, ResetInfo_t *resetInfo) const;
	//-----------------------------------------------------------------------------
	// Old function will be deprecated
	//-----------------------------------------------------------------------------
  public:
	void initialize_amfitrack();
	void start_amfitrack_task(void);
	void stop_amfitrack_task(void);
	void amfitrack_main_loop(void);

	bool getDeviceActive(uint8_t DeviceID);
	void getDevicePose(uint8_t DeviceID, lib_AmfiProt_Amfitrack_Pose_t *Pose);
	void getDeviceIMU(uint8_t DeviceID, lib_AmfiProt_Amfitrack_IMU_t *imuData);
	void getSensorMeasurements(uint8_t DeviceID, lib_AmfiProt_Amfitrack_Sensor_Measurement_t *SensorMeasurement);
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
	void getSensorTimestamp(uint8_t DeviceID, std::chrono::steady_clock::time_point *time_stamp);
#endif
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------

  private:
	AMFITRACK() = default;
	~AMFITRACK() = default;

	AMFITRACK(AMFITRACK const &) = delete;
	AMFITRACK &operator=(AMFITRACK const &) = delete;

	static void background_amfitrack_task(AMFITRACK *);
};

#endif
/** @} */ // end of module
