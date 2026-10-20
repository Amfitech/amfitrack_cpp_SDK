//-----------------------------------------------------------------------------
//
//                              AMFITECH APS
//
//                          ALL RIGHTS RESERVED
//
//-----------------------------------------------------------------------------
#pragma once
//-----------------------------------------------------------------------------
// Section: Includes
//-----------------------------------------------------------------------------
#include "hidapi.h"
#include <cstdint>
#include <functional>
#include <vector>
#include "lib_AmfiProt_API.hpp"
#include "Amfitrack_Sensor.h"
#include "Amfitrack_Source.h"

#ifdef USE_THREAD_BASED
#include <mutex>
#endif

//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------
static constexpr uint16_t VID = 0x0C17;
static constexpr uint16_t PID_Source = 0x0D01;
static constexpr uint16_t PID_Sensor = 0x0D12;
static constexpr size_t USB_REPORT_LENGTH = 64;
static constexpr size_t MAX_NAME_LENGTH_ = 32;
static constexpr uint8_t kUSBReportId = 0x01;

//-----------------------------------------------------------------------------
// Section: Typedef
//-----------------------------------------------------------------------------
struct HIDMonitorCallbacks
{
	std::function<bool(size_t &lenOut,
					   uint8_t &txIdOut,
					   void *&dataOut)>
		txPoll;

	std::function<void(bool removeFromQueue)> txDone;

	std::function<void(uint8_t sourceAddres, const uint8_t *data, size_t len)> rxPush;
};
//-----------------------------------------------------------------------------
// Section: Macro
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Class
//-----------------------------------------------------------------------------
class AMFITRACK_HID
{
  public:
	uint8_t deviceId;
	char name[MAX_NAME_LENGTH];
	uint32_t uuid[3];

	hid_device *_dev_handle;
};

class HIDMonitor
{
  public:
	explicit HIDMonitor(HIDMonitorCallbacks callbacks);
	~HIDMonitor();

	bool init();
	void run();
	bool shutdown();

	void set_hid_device(uint8_t deviceID, hid_device *handle);

  private:
	void syncDevices();
	void scanForPid(uint16_t pid);
	void removeDisconnected();

	// Separate probe per type since they have different fields to fill
	bool probeSensorIdentity(AMFITRACK_HID &sensor);
	bool probeSourceIdentity(AMFITRACK_HID &source);

	void drainTxQueue();
	void drainRx();

	hid_device *findHandleByTxId(uint8_t txId);

	int hidWrite(hid_device *dev, const void *data, size_t len);
	int hidReadNonBlocking(hid_device *dev, void *data);
	int hidReadTimeout(hid_device *dev, void *data, int timeoutMs);

	HIDMonitorCallbacks _cb;

	bool _initialized = false;
	uint32_t _lastScanTime = 0;

#ifdef USE_THREAD_BASED
	mutable std::mutex _mutex;
#endif
};
