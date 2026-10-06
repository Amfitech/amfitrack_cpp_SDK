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
#include "Amfitrack_Devices.h"

#include "Amfitrack_Sensor.h"
#include "lib_log.h"
#include "lib_time.h"

#include <algorithm>
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

AMFITRACK_Devices &AMFITRACK_Devices::getInstance()
{
	static AMFITRACK_Devices instance;
	return instance;
}

AMFITRACK_Devices::AMFITRACK_Devices()
{
}

uint32_t AMFITRACK_Devices::device_count()
{
	return AMFITRACK_DEVICE_COUNT;
}

bool AMFITRACK_Devices::is_valid_device_id(uint8_t device_id)
{
	return (device_id < AMFITRACK_DEVICE_COUNT) &&
		   (device_id != AMFITRACK_BROADCAST_DEVICE_ID);
}

AMFITRACK_Devices::deviceType_t AMFITRACK_Devices::device_id_exist(uint8_t device_id)
{
	const bool sensorExists = (_sensors.find(device_id) != _sensors.end());
	const bool sourceExists = (_sources.find(device_id) != _sources.end());

	if (sensorExists)
	{
		return deviceType_t::Sensor;
	}

	if (sourceExists)
	{
		return deviceType_t::Source;
	}

	return deviceType_t::None;
}

AMFITRACK_Source *AMFITRACK_Devices::get_or_create_source(uint8_t device_id)
{
	if (!is_valid_device_id(device_id))
	{
		return nullptr;
	}

	auto result = _sources.emplace(device_id, AMFITRACK_Source(device_id));
	return &result.first->second;
}

AMFITRACK_Sensor *AMFITRACK_Devices::get_or_create_sensor(uint8_t device_id)
{
	if (!is_valid_device_id(device_id))
	{
		return nullptr;
	}

	auto result = _sensors.emplace(device_id, AMFITRACK_Sensor(device_id));
	return &result.first->second;
}

bool AMFITRACK_Devices::get_sensor_by_id(uint8_t device_id, AMFITRACK_Sensor *sensor)
{
	if ((sensor == nullptr) || !is_valid_device_id(device_id))
	{
		AMFITRACK_Sensor emptySensor;
		*sensor = emptySensor;
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (device_id_exist(device_id) != deviceType_t::Sensor)
	{
		AMFITRACK_Sensor emptySensor;
		*sensor = emptySensor;
		return false;
	}

	*sensor = _sensors[device_id];
	return true;
}

bool AMFITRACK_Devices::get_source_by_id(uint8_t device_id, AMFITRACK_Source *source)
{
	if ((source == nullptr) || !is_valid_device_id(device_id))
	{
		AMFITRACK_Source emptySource;
		*source = emptySource;
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (device_id_exist(device_id) != deviceType_t::Source)
	{
		AMFITRACK_Source emptySource;
		*source = emptySource;
		return false;
	}

	*source = _sources[device_id];
	return true;
}

bool AMFITRACK_Devices::get_sensor_by_number(uint8_t device_number, AMFITRACK_Sensor *sensor)
{
	if (sensor == nullptr)
	{
		AMFITRACK_Sensor emptySensor;
		*sensor = emptySensor;
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (device_number >= _sensors.size())
	{
		AMFITRACK_Sensor emptySensor;
		*sensor = emptySensor;
		return false;
	}

	auto it = _sensors.begin();
	std::advance(it, device_number);

	*sensor = it->second;
	return true;
}

bool AMFITRACK_Devices::get_source_by_number(uint8_t device_number, AMFITRACK_Source *source)
{
	if (source == nullptr)
	{
		AMFITRACK_Source emptySource;
		*source = emptySource;
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (device_number >= _sources.size())
	{
		AMFITRACK_Source emptySource;
		*source = emptySource;
		return false;
	}

	auto it = _sources.begin();
	std::advance(it, device_number);

	*source = it->second;
	return true;
}

uint8_t AMFITRACK_Devices::get_numer_of_sensors()
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	return static_cast<uint8_t>(_sensors.size());
}

uint8_t AMFITRACK_Devices::get_numer_of_sources()
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	return static_cast<uint8_t>(_sources.size());
}

bool AMFITRACK_Devices::is_device_active(uint8_t device_id)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

	bool isActive = false;

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif
	deviceType_t type = device_id_exist(device_id);
	if (type == deviceType_t::Sensor)
	{
		isActive = _sensors.at(device_id).active;
	}
	else if (type == deviceType_t::Source)
	{
		isActive = _sources.at(device_id).active;
	}

	return isActive;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, bool isActive)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	if (newType == deviceType_t::Both || newType == deviceType_t::None)
		newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		auto it = _sensors.find(device_id);

		if (isActive)
		{
			AMFITRACK_Sensor *sensor = get_or_create_sensor(device_id);

			if (sensor->active != true)
			{
				LOG_I("Device %u connected!", device_id);
			}

			sensor->active = true;
			update_last_seen(device_id, newType);
		}
		else
		{
			if (it != _sensors.end())
			{
				LOG_I("Device %u disconnected!", device_id);
				_sensors.erase(it);
			}
		}
	}
	else if (newType == deviceType_t::Source)
	{
		auto it = _sources.find(device_id);

		if (isActive)
		{
			AMFITRACK_Source *source = get_or_create_source(device_id);

			if (source->active != true)
			{
				LOG_I("Device %u connected!", device_id);
			}

			source->active = true;
			update_last_seen(device_id, newType);
		}
		else
		{
			if (it != _sources.end())
			{
				LOG_I("Device %u disconnected!", device_id);
				_sources.erase(it);
			}
		}
	}

	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, char const *name, uint8_t length)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	if (newType == deviceType_t::Both || newType == deviceType_t::None)
		newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		if ((name != nullptr) && (length > 0))
		{
			const std::size_t copy_len = std::min<std::size_t>(length, MAX_NAME_LENGTH - 1);
			std::memcpy(_sensors[device_id].name, name, copy_len);
			_sensors[device_id].name[copy_len] = '\0';
		}
		else
		{
			_sensors[device_id].name[0] = '\0';
		}
	}
	else if (newType == deviceType_t::Source)
	{
		if ((name != nullptr) && (length > 0))
		{
			const std::size_t copy_len = std::min<std::size_t>(length, MAX_NAME_LENGTH - 1);
			std::memcpy(_sources[device_id].name, name, copy_len);
			_sources[device_id].name[copy_len] = '\0';
		}
		else
		{
			_sources[device_id].name[0] = '\0';
		}
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, uint32_t UUID1, uint32_t UUID2, uint32_t UUID3)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	if (newType == deviceType_t::Both || newType == deviceType_t::None)
		newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		_sensors[device_id].uuid[0] = UUID1;
		_sensors[device_id].uuid[1] = UUID2;
		_sensors[device_id].uuid[2] = UUID3;
	}
	else if (newType == deviceType_t::Source)
	{
		_sources[device_id].uuid[0] = UUID1;
		_sources[device_id].uuid[1] = UUID2;
		_sources[device_id].uuid[2] = UUID3;
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, ResetInfoLog_t const &resetInfo)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	if (newType == deviceType_t::Both || newType == deviceType_t::None)
		newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		_sensors[device_id].resetInfo = resetInfo;
	}
	else if (newType == deviceType_t::Source)
	{
		_sources[device_id].resetInfo = resetInfo;
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, FW_t fwVersion)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	if (newType == deviceType_t::Both || newType == deviceType_t::None)
		newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		_sensors[device_id].FW_Version = fwVersion;
	}
	else if (newType == deviceType_t::Source)
	{
		_sources[device_id].FW_Version = fwVersion;
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, RF_t rfVersion)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	if (newType == deviceType_t::Both || newType == deviceType_t::None)
		newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		_sensors[device_id].RF_Version = rfVersion;
	}
	else if (newType == deviceType_t::Source)
	{
		_sources[device_id].RF_Version = rfVersion;
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, HW_t hwVersion)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	if (newType == deviceType_t::Both || newType == deviceType_t::None)
		newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		_sensors[device_id].HW_Version = hwVersion;
	}
	else if (newType == deviceType_t::Source)
	{
		_sources[device_id].HW_Version = hwVersion;
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, uint8_t hubId)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	if (newType == deviceType_t::Both || newType == deviceType_t::None)
		newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		_sensors[device_id].hub_ID = hubId;
	}
	else if (newType == deviceType_t::Source)
	{
		_sources[device_id].hub_ID = hubId;
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set_hid(uint8_t device_id, deviceType_t type, hid_device *hidHandle)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (type == deviceType_t::Sensor)
	{
		_sensors[device_id]._dev_handle = hidHandle;
	}
	else
	{
		_sources[device_id]._dev_handle = hidHandle;
	}

	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, DeviceConfig_t const &config)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		_sensors[device_id].config = config;
	}
	else if (newType == deviceType_t::Source)
	{
		_sources[device_id].config = config;
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, deviceType_t type, IMU_t const &imu)
{
	deviceType_t newType = type;
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if ((newType == deviceType_t::Both || newType == deviceType_t::None) && device_id_exist(device_id) == deviceType_t::None)
		return false;

	newType = device_id_exist(device_id);

	if (newType == deviceType_t::Sensor)
	{
		_sensors[device_id].imu = imu;
	}
	else if (newType == deviceType_t::Source)
	{
		_sources[device_id].imu = imu;
	}

	update_last_seen(device_id, newType);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Sensor_Status_t const &status)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sensors[device_id].status = status;
	update_last_seen(device_id, deviceType_t::Sensor);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, External_input_t const &ext_input)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sensors[device_id].external_input = ext_input;
	update_last_seen(device_id, deviceType_t::Sensor);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Pose_t const &pose)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sensors[device_id].pose = pose;
	update_last_seen(device_id, deviceType_t::Sensor);
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
	_sensors[device_id].timestamp = std::chrono::steady_clock::now();
#endif
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Raw_B_Field_t const &rawBfield)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sensors[device_id].rawBfield = rawBfield;
	update_last_seen(device_id, deviceType_t::Sensor);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Normalized_B_Field_t const &normBfield)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sensors[device_id].normBfield = normBfield;
	update_last_seen(device_id, deviceType_t::Sensor);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Raw_with_Phase_B_Field_t const &rawWithPhaseBfield)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sensors[device_id].rawWithPhaseBfield = rawWithPhaseBfield;
	update_last_seen(device_id, deviceType_t::Sensor);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, lib_AmfiProt_Amfitrack_Sensor_Measurement_t const &sensorMeasurement)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sensors[device_id].sensorMeasurement = sensorMeasurement;
	update_last_seen(device_id, deviceType_t::Sensor);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Source_Status_t const &status)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sources[device_id].status = status;
	update_last_seen(device_id, deviceType_t::Source);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Current_t const &current)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sources[device_id].current = current;
	update_last_seen(device_id, deviceType_t::Source);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Frequency_t const &frequency)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sources[device_id].frequency = frequency;
	update_last_seen(device_id, deviceType_t::Source);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Voltage_t const &voltage)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sources[device_id].voltage = voltage;
	update_last_seen(device_id, deviceType_t::Source);
	return true;
}

bool AMFITRACK_Devices::set(uint8_t device_id, Calibration_t const &calibration)
{
	if (!is_valid_device_id(device_id))
	{
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	_sources[device_id].calibration = calibration;
	update_last_seen(device_id, deviceType_t::Source);
	return true;
}

void AMFITRACK_Devices::update_last_seen(uint8_t device_id, deviceType_t type)
{
	const uint32_t now = lib_time::get_time_ms();

	if (type == deviceType_t::Sensor)
	{
		AMFITRACK_Sensor *sensor = get_or_create_sensor(device_id);
		if (sensor->active != true)
		{
			LOG_I("Device %u connected!", device_id);
		}
		sensor->lastTimeSeenMs = now;
		sensor->active = true;
		sensor->deviceId = device_id;
	}
	else if (type == deviceType_t::Source)
	{
		AMFITRACK_Source *source = get_or_create_source(device_id);
		if (source->active != true)
		{
			LOG_I("Device %u connected!", device_id);
		}
		source->lastTimeSeenMs = now;
		source->active = true;
		source->deviceId = device_id;
	}
}
