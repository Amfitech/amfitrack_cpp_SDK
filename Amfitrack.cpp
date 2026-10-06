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
#include "Amfitrack.h"

#include "lib_AmfiProt_API.hpp"
#include "lib_log.h"
#include "Amfitrack_config.h"
#include "Amfitrack_resetinfo.h"
#include "Amfitrack_Devices.h"
#include "Amfitrack_Source.h"
#include "Amfitrack_task.h"
#include "HID_Monitor.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <utility>

#ifdef USE_THREAD_BASED
#include <thread>
#include <mutex>
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
static AmfiProt_API *amfiprot_api = nullptr;
static std::unique_ptr<HIDMonitor> hid_monitor = nullptr;
static std::atomic<bool> stop_running{false};
#ifdef USE_THREAD_BASED
static std::thread background_thread;
static std::mutex background_thread_mutex;
#endif
//-----------------------------------------------------------------------------
// Section: Function prototypes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Section: Functions
//-----------------------------------------------------------------------------
static void _run_all_amfitrack()
{
#ifdef USE_USB
	hid_monitor->run();
#endif
	amfiprot_api->amfiprot_run();
	AMFITRACK_Config::getInstance().run();
	AMFITRACK_ResetInfo::getInstance().run();
	amfitrack_task::run();
}

void AMFITRACK::background_amfitrack_task(AMFITRACK *inst)
{
	(void)inst;
	while (!stop_running.load(std::memory_order_acquire))
	{
		_run_all_amfitrack();

		// Every sub-task above is non-blocking and internally rate-limited, so
		// without a pause this loop busy-spins a whole core. Pending HID reports
		// are drained in batches each pass, so a 1 ms pause adds no meaningful
		// input latency; with no devices attached only the 1 Hz USB scan needs
		// to run, so idle can sleep longer.
		const bool devices_present =
			AMFITRACK_Devices::getInstance().get_numer_of_sensors() > 0 ||
			AMFITRACK_Devices::getInstance().get_numer_of_sources() > 0;
		std::this_thread::sleep_for(std::chrono::milliseconds(devices_present ? 1 : 50));
	}
}

AMFITRACK &AMFITRACK::getInstance()
{
	static AMFITRACK instance;
	return instance;
}

void AMFITRACK::init()
{
	amfiprot_api = &AmfiProt_API::getInstance();
#if defined(USE_USB)
	if (!hid_monitor)
	{
		HIDMonitorCallbacks cb;

		cb.txPoll = [](size_t &len, uint8_t &txId, void *&data) -> bool
		{
			return amfiprot_api->isDataReadyForTransmit(&len, &txId, &data);
		};

		cb.txDone = [](bool removeFromQueue)
		{
			amfiprot_api->set_transmit_ongoing_and_check_respons_request(removeFromQueue);
		};

		cb.rxPush = [](uint8_t sourceAddress, const uint8_t *data, size_t len)
		{
			uint8_t _deviceID = data[4];
			AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Both, sourceAddress);
			amfiprot_api->deserialize_frame(data, static_cast<uint8_t>(len));
		};

		hid_monitor = std::make_unique<HIDMonitor>(std::move(cb));
	}
	hid_monitor->init();
#endif
	amfitrack_task::init();
}

void AMFITRACK::start_task()
{

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(background_thread_mutex);
	if (background_thread.joinable())
	{
		return;
	}

	stop_running.store(false, std::memory_order_release);
	background_thread = std::thread(background_amfitrack_task, this);
#endif
}

void AMFITRACK::stop_task()
{
	stop_running.store(true, std::memory_order_release);
#ifdef USE_THREAD_BASED
	std::thread thread_to_join;
	{
		const std::lock_guard<std::mutex> lock(background_thread_mutex);
		if (background_thread.joinable())
		{
			thread_to_join = std::move(background_thread);
		}
	}

	if (thread_to_join.joinable())
	{
		thread_to_join.join();
	}
#endif
#ifdef USE_USB
	if (hid_monitor)
	{
		hid_monitor->shutdown();
	}
#endif
}

void AMFITRACK::run()
{
	_run_all_amfitrack();
}

bool AMFITRACK::get_sensor(uint8_t device_id, AMFITRACK_Sensor *sensor) const
{
	return AMFITRACK_Devices::getInstance().get_sensor_by_id(device_id, sensor);
}

bool AMFITRACK::get_source(uint8_t device_id, AMFITRACK_Source *source) const
{
	return AMFITRACK_Devices::getInstance().get_source_by_id(device_id, source);
}

bool AMFITRACK::get_sensor_by_number(uint8_t device_number, AMFITRACK_Sensor *sensor) const
{
	return AMFITRACK_Devices::getInstance().get_sensor_by_number(device_number, sensor);
}

bool AMFITRACK::get_source_by_number(uint8_t device_number, AMFITRACK_Source *source) const
{
	return AMFITRACK_Devices::getInstance().get_source_by_number(device_number, source);
}

uint8_t AMFITRACK::get_sensors_active() const
{
	return AMFITRACK_Devices::getInstance().get_numer_of_sensors();
}

uint8_t AMFITRACK::get_sources_active() const
{
	return AMFITRACK_Devices::getInstance().get_numer_of_sources();
}

bool AMFITRACK::setConfiguration(uint8_t DeviceID, uint32_t UID, lib_Generic_Parameter_Value_t parameter)
{
	return AMFITRACK_Config::getInstance().setConfiguration(DeviceID, UID, parameter);
}

bool AMFITRACK::getConfiguration(uint8_t DeviceID, bool force_all)
{
	return AMFITRACK_Config::getInstance().start(DeviceID, force_all);
}

ConfigDiscoveryState_t AMFITRACK::getConfigurationState(uint8_t DeviceID) const
{
	return AMFITRACK_Config::getInstance().state(DeviceID);
}

bool AMFITRACK::requestResetInfo(uint8_t DeviceID, uint8_t RecordCount)
{
	return AMFITRACK_ResetInfo::getInstance().start(DeviceID, RecordCount);
}

ResetInfoState_t AMFITRACK::getResetInfoState(uint8_t DeviceID) const
{
	return AMFITRACK_ResetInfo::getInstance().state(DeviceID);
}

bool AMFITRACK::getResetInfoLog(uint8_t DeviceID, ResetInfoLog_t *resetInfoLog) const
{
	return AMFITRACK_ResetInfo::getInstance().get(DeviceID, resetInfoLog);
}

bool AMFITRACK::getResetInfo(uint8_t DeviceID, uint8_t RecordIndex, ResetInfo_t *resetInfo) const
{
	return AMFITRACK_ResetInfo::getInstance().get(DeviceID, RecordIndex, resetInfo);
}

//-----------------------------------------------------------------------------
// Old function will be deprecated
//-----------------------------------------------------------------------------
void AMFITRACK::initialize_amfitrack()
{
	init();
}
void AMFITRACK::start_amfitrack_task(void)
{
	start_task();
}
void AMFITRACK::stop_amfitrack_task(void)
{
	stop_task();
}
void AMFITRACK::amfitrack_main_loop(void)
{
	run();
}

bool AMFITRACK::getDeviceActive(uint8_t DeviceID)
{
	AMFITRACK_Sensor sensor;
	AMFITRACK_Devices::getInstance().get_sensor_by_id(DeviceID, &sensor);
	return sensor.active;
}
void AMFITRACK::getDevicePose(uint8_t DeviceID, lib_AmfiProt_Amfitrack_Pose_t *Pose)
{
	AMFITRACK_Sensor sensor;
	AMFITRACK_Devices::getInstance().get_sensor_by_id(DeviceID, &sensor);
	Pose->position_x_in_m = sensor.pose.Position_X;
	Pose->position_y_in_m = sensor.pose.Position_Y;
	Pose->position_z_in_m = sensor.pose.Position_Z;

	Pose->orientation_x = sensor.pose.Orientation_X;
	Pose->orientation_y = sensor.pose.Orientation_Y;
	Pose->orientation_z = sensor.pose.Orientation_Z;
	Pose->orientation_w = sensor.pose.Orientation_W;
}
void AMFITRACK::getDeviceIMU(uint8_t DeviceID, lib_AmfiProt_Amfitrack_IMU_t *imuData)
{
	AMFITRACK_Sensor sensor;
	AMFITRACK_Devices::getInstance().get_sensor_by_id(DeviceID, &sensor);
	imuData->acceleration_x_in_mg = sensor.imu.Acceleration_X / 1000.0f;
	imuData->acceleration_x_in_mg = sensor.imu.Acceleration_X / 1000.0f;
	imuData->acceleration_x_in_mg = sensor.imu.Acceleration_X / 1000.0f;

	imuData->rotation_x_in_rad_per_sec = sensor.imu.Rotation_X;
	imuData->rotation_y_in_rad_per_sec = sensor.imu.Rotation_Y;
	imuData->rotation_z_in_rad_per_sec = sensor.imu.Rotation_Z;
}
void AMFITRACK::getSensorMeasurements(uint8_t DeviceID, lib_AmfiProt_Amfitrack_Sensor_Measurement_t *SensorMeasurement)
{
	AMFITRACK_Sensor sensor;
	AMFITRACK_Devices::getInstance().get_sensor_by_id(DeviceID, &sensor);
	SensorMeasurement = &sensor.sensorMeasurement;
}

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
void AMFITRACK::getSensorTimestamp(uint8_t DeviceID, std::chrono::steady_clock::time_point *time_stamp)
{
	AMFITRACK_Sensor sensor;
	AMFITRACK_Devices::getInstance().get_sensor_by_id(DeviceID, &sensor);
	time_stamp = &sensor.timestamp;
}
#endif
