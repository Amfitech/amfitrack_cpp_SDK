//-----------------------------------------------------------------------------
//                              AMFITECH APS
//                          ALL RIGHTS RESERVED
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Section: Includes
//-----------------------------------------------------------------------------
#include "AmfitrackDeviceTypes.h"
#include "lib_AmfiProt.hpp"

#include <cstdint>

#ifdef USE_THREAD_BASED
#include <mutex>
#endif

//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Typedef
//-----------------------------------------------------------------------------
typedef enum
{
	CONFIG_DISCOVERY_IDLE,
	CONFIG_DISCOVERY_CATEGORY_COUNT,
	CONFIG_DISCOVERY_CATEGORY_NAMES,
	CONFIG_DISCOVERY_VALUE_COUNT,
	CONFIG_DISCOVERY_NAMES_BY_UID,
	CONFIG_DISCOVERY_VALUES_BY_UID,
	CONFIG_DISCOVERY_DONE,
} ConfigDiscoveryState_t;

//-----------------------------------------------------------------------------
// Section: Macro
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Class
//-----------------------------------------------------------------------------

class AMFITRACK_Config
{

  public:
	static AMFITRACK_Config &getInstance();

	bool start(uint8_t device_id, bool force_all = false);
	void run();
	ConfigDiscoveryState_t state(uint8_t device_id) const;
	bool setConfiguration(uint8_t DeviceID, uint32_t UID, lib_Generic_Parameter_Value_t parameter);

	bool set(uint8_t device_id, lib_AmfiProt_ConfigCategoryCount_t const &category_count);
	bool set(uint8_t device_id, lib_AmfiProt_ConfigCategory_t const &category);
	bool set(uint8_t device_id, lib_AmfiProt_ConfigValueCount_t const &config_count);
	bool set(uint8_t device_id, lib_AmfiProt_ConfigNameUID_protocol_t const &config_name);
	bool set(uint8_t device_id, lib_AmfiProt_ConfigValueUID_t const &config_value);

	void print_config(uint8_t device_id);

  private:
	AMFITRACK_Config() = default;
	~AMFITRACK_Config() = default;

	AMFITRACK_Config(AMFITRACK_Config const &) = delete;
	AMFITRACK_Config &operator=(AMFITRACK_Config const &) = delete;

	bool request_current();
	bool request_category_count();
	bool request_category_name();
	bool request_value_count();
	bool request_name_by_uid();
	bool request_value_by_uid();

	void set_waiting_for_reply(bool waiting);
	bool select_next_config(DeviceConfig_t const &config);
	bool is_active(uint8_t device_id, ConfigDiscoveryState_t state) const;
	void advance_to_value_count(DeviceConfig_t const &config);
	void advance_to_names_by_uid(DeviceConfig_t const &config);
	void advance_to_values_by_uid(DeviceConfig_t const &config);
	void finish();

#ifdef USE_THREAD_BASED
	mutable std::mutex _mutex;
#endif

	uint8_t _device_id = 0U;
	ConfigDiscoveryState_t _state = CONFIG_DISCOVERY_IDLE;
	uint8_t _category_index = 0U;
	uint16_t _config_index = 0U;
	bool _force_all_config = false;
	bool _waiting_for_reply = false;
	uint32_t _last_request_time = 0;
};
