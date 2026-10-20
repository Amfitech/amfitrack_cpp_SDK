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
#include "HID_Monitor.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include "lib_AmfiProt.hpp"
#include "lib_log.h"
#include "lib_time.h"
#include "Amfitrack.h"
#include "Amfitrack_Devices.h"
#include "Amfitrack_Sensor.h"
#include "Amfitrack_Source.h"

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
static constexpr int kProbeTimeoutMs = 10;
static constexpr int kProbeMaxAttempts = 10;
static constexpr uint32_t kScanIntervalMs = 1000;

struct PendingHIDDevice
{
	uint16_t pid;
	uint32_t firstSeen;
};

std::unordered_map<std::string, PendingHIDDevice> _pendingDevices;
static constexpr uint32_t kProbeDelayMs = 500;

//-----------------------------------------------------------------------------
// Section: Function prototypes
//-----------------------------------------------------------------------------
static bool isSameDevice(hid_device *handle, const hid_device_info *info)
{
	if (!handle || !info)
		return false;
	const hid_device_info *cur = hid_get_device_info(handle);
	if (!cur)
		return false;
	if (cur->serial_number && info->serial_number)
		return std::wcscmp(cur->serial_number, info->serial_number) == 0;
	if (cur->path && info->path)
		return std::strcmp(cur->path, info->path) == 0;
	return false;
}

static bool parseNameReply(const lib_AmfiProt_Frame_t &f, char *buf, size_t bufSize)
{
	if (!buf || bufSize == 0)
		return false;
	if (f.header.payloadType != libAmfiProt_PayloadType_Common ||
		f.payload[0] != lib_AmfiProt_PayloadID_ReplyDeviceName)
		return false;
	const size_t len = strnlen(reinterpret_cast<const char *>(&f.payload[1]), bufSize - 1);
	std::memcpy(buf, &f.payload[1], len);
	buf[len] = '\0';
	return true;
}

// Shared probe logic — used by both probeSensorIdentity and probeSourceIdentity
static bool probeDeviceIdentity(hid_device *handle,
								std::function<int(hid_device *, const void *, size_t)> writeFn,
								std::function<int(hid_device *, void *, int)> readFn,
								uint8_t &deviceIdOut,
								uint32_t uuidOut[3],
								char *nameOut,
								size_t nameSize,
								const char *requiredNamePart = nullptr)
{
	AmfiProt_API &api = AmfiProt_API::getInstance();
	lib_AmfiProt_Frame_t txFrame{}, rxFrame{};
	uint8_t packet[USB_REPORT_LENGTH]{};

	// ── Request name via broadcast ───────────────────────────────────────────
	uint8_t payload = lib_AmfiProt_PayloadID_RequestDeviceName;

	api.lib_AmfiProt_EncodeFrame(&txFrame,
								 &payload,
								 sizeof(payload),
								 libAmfiProt_PayloadType_Common,
								 0,
								 lib_AmfiProt_destination_Broadcast,
								 lib_AmfiProt_packetType_NoAck);

	std::memcpy(packet, &txFrame, api.lib_AmfiProt_FrameSize(&txFrame));

	if (writeFn(handle, packet, api.lib_AmfiProt_FrameSize(&txFrame)) < 0)
		return false;

	for (int i = 0; i < kProbeMaxAttempts; ++i)
	{
		const int n = readFn(handle, packet, kProbeTimeoutMs);
		if (n <= 0)
			continue;

		api.deserialize_frame(packet, static_cast<uint8_t>(n));

		if (!api.lib_AmfiProt_DeserializeFrame(&rxFrame, packet, static_cast<uint8_t>(n)))
			continue;

		if (!parseNameReply(rxFrame, nameOut, nameSize))
			continue;

		// Device ID is taken from the received frame header
		deviceIdOut = rxFrame.header.source;

		// UUID is no longer requested here
		uuidOut[0] = 0;
		uuidOut[1] = 0;
		uuidOut[2] = 0;

		// Optional name filter, e.g. "source"
		if (requiredNamePart != nullptr)
		{
			if (std::strstr(nameOut, requiredNamePart) == nullptr)
				continue;
		}

		return true;
	}

	return false;
}

bool stillPresent(hid_device *handle, uint16_t pid)
{
	if (!handle)
		return false;
	hid_device_info *list = hid_enumerate(VID, pid);
	bool found = false;
	for (const hid_device_info *info = list; info && !found; info = info->next)
		found = isSameDevice(handle, info);
	hid_free_enumeration(list);
	return found;
}
//-----------------------------------------------------------------------------
// Section: Functions
//-----------------------------------------------------------------------------
HIDMonitor::HIDMonitor(HIDMonitorCallbacks callbacks)
	: _cb(std::move(callbacks))
{
}

HIDMonitor::~HIDMonitor()
{
	shutdown();
}

bool HIDMonitor::init()
{
#ifdef USE_THREAD_BASED
	std::lock_guard<std::mutex> lock(_mutex);
#endif
	if (_initialized)
		return true;
	if (hid_init() != 0)
	{
		LOG_E("hid_init() failed");
		return false;
	}
	_initialized = true;
	syncDevices();
	return true;
}

void HIDMonitor::run()
{
#ifdef USE_THREAD_BASED
	std::lock_guard<std::mutex> lock(_mutex);
#endif
	if (!_initialized)
		return;
	syncDevices();
	drainTxQueue();
	drainRx();
}

bool HIDMonitor::shutdown()
{
#ifdef USE_THREAD_BASED
	std::lock_guard<std::mutex> lock(_mutex);
#endif
	if (!_initialized)
	{
		return true;
	}

	for (uint8_t i = 0; i < AMFITRACK::getInstance().get_sensors_active(); i++)
	{
		AMFITRACK_Sensor s;
		AMFITRACK::getInstance().get_sensor_by_number(i, &s);
		if (s._dev_handle)
		{
			hid_close(s._dev_handle);
			AMFITRACK_Devices::getInstance().set_hid(s.deviceId, AMFITRACK_Devices::deviceType_t::Sensor, nullptr);
		}
	}
	for (uint8_t i = 0; i < AMFITRACK::getInstance().get_sources_active(); i++)
	{
		AMFITRACK_Source s;
		AMFITRACK::getInstance().get_source_by_number(i, &s);
		if (s._dev_handle)
		{
			hid_close(s._dev_handle);
			AMFITRACK_Devices::getInstance().set_hid(s.deviceId, AMFITRACK_Devices::deviceType_t::Source, nullptr);
		}
	}
	hid_exit();
	_initialized = false;
	return true;
}

void HIDMonitor::syncDevices()
{
	const uint32_t now = lib_time::get_time_ms();

	if (_lastScanTime != 0 &&
		(now - _lastScanTime) < kScanIntervalMs)
	{
		return;
	}

	scanForPid(PID_Sensor);
	scanForPid(PID_Source);
	removeDisconnected();
	_lastScanTime = now;
}

bool isAlreadyOpen(uint16_t pid, const hid_device_info *info)
{
	if (pid == PID_Sensor)
	{
		for (uint8_t i = 0; i < AMFITRACK::getInstance().get_sensors_active(); i++)
		{
			AMFITRACK_Sensor _sensors;
			AMFITRACK::getInstance().get_sensor_by_number(i, &_sensors);
			if (_sensors._dev_handle)
			{
				if (isSameDevice(_sensors._dev_handle, info))
				{
					return true;
				}
			}
		}
	}
	else if (pid == PID_Source)
	{
		for (uint8_t i = 0; i < AMFITRACK::getInstance().get_sources_active(); i++)
		{
			AMFITRACK_Source _sources;
			AMFITRACK::getInstance().get_source_by_number(i, &_sources);
			if (isSameDevice(_sources._dev_handle, info))
			{
				return true;
			}
		}
	}
	return false;
}

void HIDMonitor::scanForPid(uint16_t pid)
{
	const uint32_t now = lib_time::get_time_ms();

	hid_device_info *list = hid_enumerate(VID, pid);

	for (const hid_device_info *info = list; info; info = info->next)
	{
		std::string path = info->path;

		if (isAlreadyOpen(pid, info))
			continue;

		auto it = _pendingDevices.find(path);

		if (it == _pendingDevices.end())
		{
			_pendingDevices[path] = PendingHIDDevice();
			_pendingDevices[path].pid = pid;
			_pendingDevices[path].firstSeen = now;
			continue;
		}

		if (now - it->second.firstSeen < kProbeDelayMs)
			continue;

		hid_device *handle = hid_open_path(info->path);
		if (!handle)
			continue;

		hid_set_nonblocking(handle, 1);

		bool success = false;

		if (pid == PID_Sensor)
		{
			AMFITRACK_HID sensor;
			sensor._dev_handle = handle;

			if (probeSensorIdentity(sensor))
			{
				AMFITRACK_Devices::getInstance().set(sensor.deviceId, AMFITRACK_Devices::deviceType_t::Sensor, true);
				AMFITRACK_Devices::getInstance().set_hid(sensor.deviceId, AMFITRACK_Devices::deviceType_t::Sensor, sensor._dev_handle);
				LOG_I("Sensor connected on USB: id=%u name=%s", sensor.deviceId, sensor.name);
				success = true;
			}
		}
		else if (pid == PID_Source)
		{
			AMFITRACK_HID source;
			source._dev_handle = handle;

			if (probeSourceIdentity(source))
			{
				AMFITRACK_Devices::getInstance().set(source.deviceId, AMFITRACK_Devices::deviceType_t::Source, true);
				AMFITRACK_Devices::getInstance().set_hid(source.deviceId, AMFITRACK_Devices::deviceType_t::Source, source._dev_handle);
				LOG_I("Source connected on USB: id=%u name=%s", source.deviceId, source.name);
				success = true;
			}
		}

		if (!success)
			hid_close(handle);

		_pendingDevices.erase(path);
	}

	hid_free_enumeration(list);
}

void HIDMonitor::removeDisconnected()
{
	for (uint8_t i = 0; i < AMFITRACK_DEVICE_COUNT; i++)
	{
		bool sensorDisconnected = false;
		bool sourceDisconnected = false;
		AMFITRACK_Sensor sensor;
		AMFITRACK_Source source;
		AMFITRACK_Devices::getInstance().get_sensor_by_id(i, &sensor);
		AMFITRACK_Devices::getInstance().get_source_by_id(i, &source);

		if (!sensor._dev_handle || stillPresent(sensor._dev_handle, PID_Sensor))
		{
			if (!source._dev_handle || stillPresent(source._dev_handle, PID_Source))
			{
				continue;
			}
			else
			{
				sourceDisconnected = true;
			}
		}
		else
		{
			sensorDisconnected = true;
		}

		if (sensorDisconnected)
		{
			LOG_I("Sensor disconnected on USB: id=%u name=%s", sensor.deviceId, sensor.name);
			if (sensor._dev_handle)
			{
				hid_close(sensor._dev_handle);
				AMFITRACK_Devices::getInstance().set_hid(i, AMFITRACK_Devices::deviceType_t::Sensor, NULL);
				sourceDisconnected = false;
			}
		}
		if (sourceDisconnected)
		{
			LOG_I("Source disconnected on USB: id=%u name=%s", source.deviceId, source.name);
			if (source._dev_handle)
			{
				hid_close(source._dev_handle);
				AMFITRACK_Devices::getInstance().set_hid(i, AMFITRACK_Devices::deviceType_t::Source, NULL);
			}
		}
	}
}

bool HIDMonitor::probeSensorIdentity(AMFITRACK_HID &sensor)
{
	uint8_t deviceId = 0;
	uint32_t uuid[3]{};
	char name[MAX_NAME_LENGTH]{};

	auto writeFn = [this](hid_device *h, const void *d, size_t l)
	{ return hidWrite(h, d, l); };
	auto readFn = [this](hid_device *h, void *d, int t)
	{ return hidReadTimeout(h, d, t); };

	if (!probeDeviceIdentity(sensor._dev_handle, writeFn, readFn, deviceId, uuid, name, 50))
		return false;

	sensor.deviceId = deviceId;
	sensor.uuid[0] = uuid[0];
	sensor.uuid[1] = uuid[1];
	sensor.uuid[2] = uuid[2];
	std::snprintf(sensor.name, sizeof(sensor.name), "%s", name);
	return true;
}

bool HIDMonitor::probeSourceIdentity(AMFITRACK_HID &source)
{
	uint8_t deviceId = 0;
	uint32_t uuid[3]{};
	char name[MAX_NAME_LENGTH]{};

	auto writeFn = [this](hid_device *h, const void *d, size_t l)
	{ return hidWrite(h, d, l); };
	auto readFn = [this](hid_device *h, void *d, int t)
	{ return hidReadTimeout(h, d, t); };

	if (!probeDeviceIdentity(source._dev_handle, writeFn, readFn, deviceId, uuid, name, 50, "Source"))
		return false;

	source.deviceId = deviceId;
	source.uuid[0] = uuid[0];
	source.uuid[1] = uuid[1];
	source.uuid[2] = uuid[2];
	std::snprintf(source.name, sizeof(source.name), "%s", name);
	return true;
}

void HIDMonitor::drainTxQueue()
{
	if (!_cb.txPoll || !_cb.txDone)
		return;

	size_t dataLen = 0;
	uint8_t txId = 0;
	void *txData = nullptr;

	if (!_cb.txPoll(dataLen, txId, txData))
		return;

	bool sent = false;
	if (txId == 255)
	{
		for (uint8_t i = 0; i < AMFITRACK_Devices::getInstance().get_numer_of_sensors(); i++)
		{
			AMFITRACK_Sensor s;
			AMFITRACK_Devices::getInstance().get_sensor_by_number(i, &s);
			if (s._dev_handle && hidWrite(s._dev_handle, txData, dataLen) >= 0)
				sent = true;
		}
		for (uint8_t i = 0; i < AMFITRACK_Devices::getInstance().get_numer_of_sources(); i++)
		{
			AMFITRACK_Source s;
			AMFITRACK_Devices::getInstance().get_source_by_number(i, &s);
			if (s._dev_handle && hidWrite(s._dev_handle, txData, dataLen) >= 0)
				sent = true;
		}
	}
	else if (hid_device *handle = findHandleByTxId(txId))
	{
		sent = hidWrite(handle, txData, dataLen) >= 0;
	}
	else
	{
		// No route to the device - drop it from the device list
		LOG_W("TxID %u not reachable, dropping frame and marking device inactive", txId);
		AMFITRACK_Devices::getInstance().set(txId, AMFITRACK_Devices::deviceType_t::Both, false);
		_cb.txDone(true);
	}

	if (sent)
		_cb.txDone(false);
}

void HIDMonitor::drainRx()
{
	if (!_cb.rxPush)
		return;

	uint8_t packet[USB_REPORT_LENGTH]{};

	auto readFrom = [&](hid_device *handle)
	{
		while (handle)
		{
			const int n = hidReadNonBlocking(handle, packet);
			if (n <= 0)
				break;

			uint8_t sourceAddress = 0;
			for (uint8_t i = 0; i < AMFITRACK_DEVICE_COUNT; i++)
			{
				AMFITRACK_Sensor sensor;
				AMFITRACK_Source source;
				AMFITRACK_Devices::getInstance().get_sensor_by_id(i, &sensor);
				AMFITRACK_Devices::getInstance().get_source_by_id(i, &source);

				if (sensor._dev_handle == handle)
				{
					sourceAddress = sensor.deviceId;
					break;
				}
				else if (source._dev_handle == handle)
				{
					sourceAddress = source.deviceId;
					break;
				}
			}
			_cb.rxPush(sourceAddress, packet, (size_t)n);
		}
	};

	for (uint8_t i = 0; i < AMFITRACK_Devices::getInstance().get_numer_of_sensors(); i++)
	{
		AMFITRACK_Sensor s;
		AMFITRACK_Devices::getInstance().get_sensor_by_number(i, &s);
		if (s._dev_handle)
			readFrom(s._dev_handle);
	}
	for (uint8_t i = 0; i < AMFITRACK_Devices::getInstance().get_numer_of_sources(); i++)
	{
		AMFITRACK_Source s;
		AMFITRACK_Devices::getInstance().get_source_by_number(i, &s);
		if (s._dev_handle)
			readFrom(s._dev_handle);
	}
}

hid_device *
HIDMonitor::findHandleByTxId(uint8_t txId)
{
	hid_device *hidHandle = nullptr;
	for (uint8_t i = 0; i < AMFITRACK_Devices::getInstance().get_numer_of_sensors(); i++)
	{
		AMFITRACK_Sensor s;
		AMFITRACK_Devices::getInstance().get_sensor_by_number(i, &s);
		if (s.deviceId == txId)
		{
			if (!s._dev_handle)
			{
				AMFITRACK_Sensor hub;
				AMFITRACK_Source hub_source;
				AMFITRACK_Devices::getInstance().get_sensor_by_id(s.hub_ID, &hub);
				AMFITRACK_Devices::getInstance().get_source_by_id(s.hub_ID, &hub_source);
				if (hub._dev_handle)
				{
					hidHandle = hub._dev_handle;
				}
				else if (hub_source._dev_handle)
				{
					hidHandle = hub_source._dev_handle;
				}
				else
				{
					// hub_ID is only learned from the sensor's own frames, so a sensor
					// re-created by a payload handler has none yet
					for (uint8_t j = 0; j < AMFITRACK_Devices::getInstance().get_numer_of_sources(); j++)
					{
						AMFITRACK_Source src;
						AMFITRACK_Devices::getInstance().get_source_by_number(j, &src);
						if (src._dev_handle)
						{
							hidHandle = src._dev_handle;
							break;
						}
					}
				}
			}
			else
			{
				hidHandle = s._dev_handle;
			}
		}
	}
	for (uint8_t i = 0; i < AMFITRACK_Devices::getInstance().get_numer_of_sources(); i++)
	{
		AMFITRACK_Source s;
		AMFITRACK_Devices::getInstance().get_source_by_number(i, &s);
		if (s.deviceId == txId && s._dev_handle)
			hidHandle = s._dev_handle;
	}

	return hidHandle;
}

int HIDMonitor::hidWrite(hid_device *dev, const void *data, size_t len)
{
	if (!dev || !data)
		return -1;
	uint8_t buf[USB_REPORT_LENGTH]{};
	buf[0] = kUSBReportId;
	std::memcpy(buf + 1, data, std::min(len, USB_REPORT_LENGTH - 1));
	return hid_write(dev, buf, USB_REPORT_LENGTH);
}

int HIDMonitor::hidReadNonBlocking(hid_device *dev, void *data)
{
	if (!dev || !data)
		return -1;
	uint8_t raw[USB_REPORT_LENGTH]{};
	const int n = hid_read(dev, raw, USB_REPORT_LENGTH);
	if (n <= 2)
		return n;
	std::memcpy(data, raw + 2, n - 2);
	return n - 2;
}

int HIDMonitor::hidReadTimeout(hid_device *dev, void *data, int timeoutMs)
{
	if (!dev || !data)
		return -1;
	uint8_t raw[USB_REPORT_LENGTH]{};
	const int n = hid_read_timeout(dev, raw, USB_REPORT_LENGTH, timeoutMs);
	if (n <= 2)
		return n;
	std::memcpy(data, raw + 2, n - 2);
	return n - 2;
}
