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
#include "Amfitrack_config.h"

#include "Amfitrack_Devices.h"
#include "Amfitrack_Sensor.h"
#include "Amfitrack_Source.h"
#include "lib_AmfiProt_API.hpp"
#include "lib_log.h"
#include "lib_time.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <limits>

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
static void log_parameter_value(const lib_Generic_Parameter_Value &value);
//-----------------------------------------------------------------------------
// Section: Functions
//-----------------------------------------------------------------------------

namespace
{
constexpr uint8_t kAllConfigCategory = static_cast<uint8_t>(lib_AmfiProt_ConfigCategory_All);
constexpr char kAllConfigCategoryName[] = "All";
constexpr std::size_t kMaxStoredCategories = std::numeric_limits<uint8_t>::max();
constexpr std::size_t kMaxStoredConfigs = std::numeric_limits<uint16_t>::max();
static constexpr uint32_t kConfigReplyTimeoutMs = 1000; // example

void copy_payload_name(char *dest, std::size_t dest_size, char const *src, std::size_t src_size)
{
	if ((dest == nullptr) || (dest_size == 0U))
	{
		return;
	}
	dest[0] = '\0';

	if (src == nullptr)
	{
		return;
	}

	std::size_t name_length = 0U;
	while ((name_length < src_size) && (src[name_length] != '\0'))
	{
		name_length++;
	}

	const std::size_t copy_length = std::min(name_length, dest_size - 1U);
	if (copy_length > 0U)
	{
		std::memcpy(dest, src, copy_length);
	}
	dest[copy_length] = '\0';
}

void initialize_new_categories(DeviceConfig_t &config, std::size_t first_index)
{
	for (std::size_t category_index = first_index; category_index < config.categories.size(); category_index++)
	{
		config.categories[category_index] = CategoryEntry_t{};
		config.categories[category_index].index = static_cast<uint8_t>(category_index);
	}
}

void initialize_new_configs(CategoryEntry_t &category, std::size_t first_index)
{
	for (std::size_t config_index = first_index; config_index < category.configs.size(); config_index++)
	{
		category.configs[config_index] = ConfigEntry_t{};
		category.configs[config_index].categoryIndex = category.index;
	}
}

void sanitize(DeviceConfig_t &config)
{
	std::size_t category_count = std::max<std::size_t>(config.categoryCount, config.categories.size());
	if (category_count > kMaxStoredCategories)
	{
		LOG_W("sanitize: category count %u exceeds max %u, trimming",
			  (unsigned)category_count,
			  (unsigned)kMaxStoredCategories);
		category_count = kMaxStoredCategories;
	}

	const std::size_t previous_category_count = config.categories.size();
	config.categories.resize(category_count);
	initialize_new_categories(config, previous_category_count);
	config.categoryCount = static_cast<uint8_t>(config.categories.size());

	for (std::size_t category_index = 0U; category_index < config.categories.size(); category_index++)
	{
		CategoryEntry_t &category = config.categories[category_index];
		std::size_t config_count = std::max<std::size_t>(category.configCount, category.configs.size());
		if (config_count > kMaxStoredConfigs)
		{
			LOG_W("sanitize: category[%u] config count %u exceeds max %u, trimming",
				  (unsigned)category_index,
				  (unsigned)config_count,
				  (unsigned)kMaxStoredConfigs);
			config_count = kMaxStoredConfigs;
		}

		const std::size_t previous_config_count = category.configs.size();
		category.configs.resize(config_count);
		initialize_new_configs(category, previous_config_count);
		category.configCount = static_cast<uint16_t>(category.configs.size());
	}
}

DeviceConfig_t load_config(uint8_t device_id)
{
	DeviceConfig_t config = {};
	AMFITRACK_Devices &devices = AMFITRACK_Devices::getInstance();

	AMFITRACK_Sensor sensor;
	AMFITRACK_Source source;

	if (devices.get_sensor_by_id(device_id, &sensor))
	{
		config = sensor.config;
		sanitize(config);
		LOG_D("load_config: device_id=%u (sensor), categoryCount=%u", device_id, config.categoryCount);
	}
	else if (devices.get_source_by_id(device_id, &source))
	{
		config = source.config;
		sanitize(config);
		LOG_D("load_config: device_id=%u (source), categoryCount=%u", device_id, config.categoryCount);
	}
	else
	{
		LOG_W("load_config: device_id=%u not found, returning empty config", device_id);
	}

	return config;
}

bool is_all_config_category(uint8_t category)
{
	return category == kAllConfigCategory;
}

std::size_t all_config_storage_index(DeviceConfig_t const &config)
{
	for (std::size_t category_index = 0U; category_index < config.categories.size(); category_index++)
	{
		if (is_all_config_category(config.categories[category_index].index))
		{
			return category_index;
		}
	}

	return config.categories.size();
}

std::size_t storage_category_index(DeviceConfig_t const &config, uint8_t wire_category, bool force_all)
{
	if (force_all || is_all_config_category(wire_category))
	{
		return all_config_storage_index(config);
	}

	return wire_category;
}

CategoryEntry_t *ensure_category(DeviceConfig_t &config, std::size_t category_index, uint8_t category_label)
{
	if (category_index >= kMaxStoredCategories)
	{
		LOG_E("ensure_category: category_index %u out of bounds (max=%u)",
			  (unsigned)category_index,
			  (unsigned)kMaxStoredCategories);
		return nullptr;
	}

	if (config.categories.size() <= category_index)
	{
		const std::size_t previous_count = config.categories.size();
		LOG_D("ensure_category: extending category storage from %u to %u",
			  (unsigned)previous_count,
			  (unsigned)(category_index + 1U));
		config.categories.resize(category_index + 1U);
		initialize_new_categories(config, previous_count);
	}
	config.categoryCount = static_cast<uint8_t>(config.categories.size());

	CategoryEntry_t &category = config.categories[category_index];
	category.index = category_label;
	return &category;
}

CategoryEntry_t *ensure_category(DeviceConfig_t &config, uint8_t category_index)
{
	return ensure_category(config, static_cast<std::size_t>(category_index), category_index);
}

CategoryEntry_t *ensure_all_config_category(DeviceConfig_t &config)
{
	CategoryEntry_t *category = ensure_category(config, all_config_storage_index(config), kAllConfigCategory);
	if (category != nullptr)
	{
		copy_payload_name(category->name, sizeof(category->name), kAllConfigCategoryName, sizeof(kAllConfigCategoryName));
	}
	return category;
}

bool store_config(uint8_t device_id, DeviceConfig_t const &config)
{
	const bool stored = AMFITRACK_Devices::getInstance().set(device_id, AMFITRACK_Devices::deviceType_t::Both, config);
	if (!stored)
	{
		LOG_E("store_config: failed to store config for device_id=%u", device_id);
	}
	return stored;
}
} // namespace

AMFITRACK_Config &AMFITRACK_Config::getInstance()
{
	static AMFITRACK_Config instance;
	return instance;
}

bool AMFITRACK_Config::start(uint8_t device_id, bool force_all)
{
	if (!AMFITRACK_Devices::is_valid_device_id(device_id))
	{
		LOG_E("start config: invalid device_id=%u", device_id);
		return false;
	}

	if (!AMFITRACK_Devices::getInstance().is_device_active(device_id))
	{
		LOG_W("start config: device not active=%u", device_id);
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	LOG_I("start: beginning config discovery for device_id=%u, force_all=%s",
		  device_id,
		  force_all ? "true" : "false");

	DeviceConfig_t config = {};
	if (force_all)
	{
		ensure_all_config_category(config);
	}
	store_config(device_id, config);

	_device_id = device_id;
	_state = force_all ? CONFIG_DISCOVERY_VALUE_COUNT : CONFIG_DISCOVERY_CATEGORY_COUNT;
	_category_index = force_all ? static_cast<uint8_t>(all_config_storage_index(config)) : 0U;
	_config_index = 0U;
	_force_all_config = force_all;
	set_waiting_for_reply(false);

	return request_current();
}

void AMFITRACK_Config::run()
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	request_current();
}

ConfigDiscoveryState_t AMFITRACK_Config::state(uint8_t device_id) const
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (device_id != _device_id)
	{
		return CONFIG_DISCOVERY_IDLE;
	}

	return _state;
}

bool AMFITRACK_Config::setConfiguration(uint8_t DeviceID, uint32_t UID, lib_Generic_Parameter_Value_t parameter)
{
	if (!AMFITRACK_Devices::getInstance().is_device_active(DeviceID))
	{
		LOG_W("setConfiguration: Device not active: %u", DeviceID);
	}
	lib_AmfiProt_ConfigValueUID_t ConfigurationPayload = {};
	ConfigurationPayload.payloadID = lib_AmfiProt_PayloadID_SetConfigurationValueUID;
	ConfigurationPayload.uid = UID;
	ConfigurationPayload.value = parameter;
	uint8_t payloadSize = sizeof(ConfigurationPayload) - sizeof(ConfigurationPayload.value) + lib_Generic_Parameter_SizeWithType(ConfigurationPayload.value);

	return AmfiProt_API::getInstance().queue_frame(&ConfigurationPayload, payloadSize, libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, DeviceID);
}

bool AMFITRACK_Config::set(uint8_t device_id, lib_AmfiProt_ConfigCategoryCount_t const &category_count)
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	LOG_D("set(CategoryCount): device_id=%u, categoryCount=%u", device_id, category_count.categoryCount);

	DeviceConfig_t config = load_config(device_id);

	const uint8_t count = category_count.categoryCount;
	const std::size_t previous_count = config.categories.size();

	if (count < previous_count)
	{
		LOG_D("set(CategoryCount): shrinking from %u to %u categories", (unsigned)previous_count, count);
	}

	config.categories.resize(count);
	initialize_new_categories(config, previous_count);

	config.categoryCount = count;
	const bool stored = store_config(device_id, config);

	if (stored && is_active(device_id, CONFIG_DISCOVERY_CATEGORY_COUNT))
	{
		LOG_I("set(CategoryCount): advancing to CATEGORY_NAMES, count=%u", count);
		_state = CONFIG_DISCOVERY_CATEGORY_NAMES;
		_category_index = 0U;
		_config_index = 0U;
		set_waiting_for_reply(false);

		if (config.categoryCount == 0U)
		{
			LOG_I("set(CategoryCount): no categories, finishing early");
			finish();
		}
		else
		{
			request_current();
		}
	}

	return stored;
}

bool AMFITRACK_Config::set(uint8_t device_id, lib_AmfiProt_ConfigCategory_t const &category)
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	LOG_D("set(Category): device_id=%u, categoryIndex=%u, name=\"%.*s\"",
		  device_id,
		  category.categoryIndex,
		  (int)sizeof(category.name),
		  category.name);

	DeviceConfig_t config = load_config(device_id);
	CategoryEntry_t *category_entry = ensure_category(config, category.categoryIndex);

	if (category_entry == nullptr)
	{
		LOG_E("set(Category): failed to ensure category at index %u", category.categoryIndex);
		return false;
	}

	copy_payload_name(category_entry->name, sizeof(category_entry->name), category.name, sizeof(category.name));
	const bool stored = store_config(device_id, config);

	if (stored && is_active(device_id, CONFIG_DISCOVERY_CATEGORY_NAMES) && (category.categoryIndex == _category_index))
	{
		_category_index++;
		set_waiting_for_reply(false);
		LOG_D("set(Category): category name stored, advancing to category_index=%u", _category_index);

		if (_category_index >= config.categoryCount)
		{
			LOG_I("set(Category): all category names received, advancing to VALUE_COUNT");
			advance_to_value_count(config);
		}

		request_current();
	}

	return stored;
}

bool AMFITRACK_Config::set(uint8_t device_id, lib_AmfiProt_ConfigValueCount_t const &config_count)
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	LOG_D("set(ValueCount): device_id=%u, category=%u, count=%u",
		  device_id,
		  config_count.category,
		  config_count.count);

	DeviceConfig_t config = load_config(device_id);
	const std::size_t category_index = storage_category_index(config, config_count.category, _force_all_config);
	CategoryEntry_t *category = (_force_all_config || is_all_config_category(config_count.category))
									? ensure_all_config_category(config)
									: ensure_category(config, category_index);

	if (category == nullptr)
	{
		LOG_E("set(ValueCount): failed to ensure category at index %u", (unsigned)category_index);
		return false;
	}

	const uint16_t count = config_count.count;
	const std::size_t previous_count = category->configs.size();

	if (count < previous_count)
	{
		LOG_D("set(ValueCount): shrinking category %u from %u to %u configs",
			  config_count.category,
			  (unsigned)previous_count,
			  count);
	}

	category->configs.resize(count);
	initialize_new_configs(*category, previous_count);

	category->configCount = count;
	const bool stored = store_config(device_id, config);

	if (stored && is_active(device_id, CONFIG_DISCOVERY_VALUE_COUNT) &&
		(_force_all_config || (config_count.category == _category_index)))
	{
		if (_force_all_config)
		{
			_category_index = static_cast<uint8_t>(category_index + 1U);
		}
		else
		{
			_category_index++;
		}
		set_waiting_for_reply(false);
		LOG_D("set(ValueCount): value count stored, advancing to category_index=%u", _category_index);

		if (_category_index >= config.categoryCount)
		{
			LOG_I("set(ValueCount): all value counts received, advancing to NAMES_BY_UID");
			advance_to_names_by_uid(config);
		}

		request_current();
	}

	return stored;
}

bool AMFITRACK_Config::set(uint8_t device_id, lib_AmfiProt_ConfigNameUID_protocol_t const &config_name)
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	const uint8_t category_index = config_name.config_names.category;
	const uint16_t config_index = config_name.index;

	LOG_D("set(NameUID): device_id=%u, category=%u, index=%u, uid=%u, name=\"%.*s\"",
		  device_id,
		  category_index,
		  config_index,
		  config_name.config_names.uid,
		  (int)sizeof(config_name.config_names.name),
		  config_name.config_names.name);

	DeviceConfig_t config = load_config(device_id);
	const std::size_t storage_index = storage_category_index(config, category_index, _force_all_config);
	CategoryEntry_t *category = (_force_all_config || is_all_config_category(category_index))
									? ensure_all_config_category(config)
									: ensure_category(config, storage_index);

	if ((category == nullptr) || (config_index >= kMaxStoredConfigs))
	{
		LOG_E("set(NameUID): invalid category %u or config_index %u out of bounds",
			  (unsigned)storage_index,
			  config_index);
		return false;
	}

	if (category->configs.size() <= config_index)
	{
		const std::size_t previous_count = category->configs.size();
		category->configs.resize(static_cast<std::size_t>(config_index) + 1U);
		initialize_new_configs(*category, previous_count);
	}
	category->configCount = static_cast<uint16_t>(category->configs.size());

	ConfigEntry_t &config_entry = category->configs[config_index];
	config_entry.uid = config_name.config_names.uid;
	config_entry.categoryIndex = category->index;
	copy_payload_name(config_entry.name, sizeof(config_entry.name), config_name.config_names.name, sizeof(config_name.config_names.name));

	const bool stored = store_config(device_id, config);

	if (stored && is_active(device_id, CONFIG_DISCOVERY_NAMES_BY_UID) &&
		(_force_all_config || (category_index == _category_index)) && (config_index == _config_index))
	{
		_config_index++;
		set_waiting_for_reply(false);
		LOG_D("set(NameUID): name stored, advancing to config_index=%u", _config_index);

		if (!select_next_config(config))
		{
			LOG_I("set(NameUID): all names by UID received, advancing to VALUES_BY_UID");
			advance_to_values_by_uid(config);
		}

		request_current();
	}

	return stored;
}

bool AMFITRACK_Config::set(uint8_t device_id, lib_AmfiProt_ConfigValueUID_t const &config_value)
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	LOG_D("set(ValueUID): device_id=%u, uid=%u", device_id, config_value.uid);

	DeviceConfig_t config = load_config(device_id);
	bool found = false;

	for (std::size_t category_index = 0U; category_index < config.categoryCount; category_index++)
	{
		CategoryEntry_t &category = config.categories[category_index];

		for (std::size_t config_index = 0U; config_index < category.configCount; config_index++)
		{
			ConfigEntry_t &config_entry = category.configs[config_index];
			if (config_entry.uid == config_value.uid)
			{
				LOG_D("set(ValueUID): matched uid=%u at category=%u, index=%u",
					  config_value.uid,
					  (unsigned)category_index,
					  (unsigned)config_index);
				config_entry.config = config_value.value;
				found = true;
			}
		}
	}

	if (!found)
	{
		LOG_I("set(ValueUID): uid=%u from device, but not stored in local database for device_id=%u",
			  config_value.uid,
			  device_id);
		log_parameter_value(config_value.value);
		return false;
	}

	const bool stored = store_config(device_id, config);

	if (stored && is_active(device_id, CONFIG_DISCOVERY_VALUES_BY_UID) &&
		(_category_index < config.categoryCount) &&
		(_config_index < config.categories[_category_index].configCount) &&
		(config.categories[_category_index].configs[_config_index].uid == config_value.uid))
	{
		_config_index++;
		set_waiting_for_reply(false);
		LOG_D("set(ValueUID): value stored, advancing to config_index=%u", _config_index);

		if (!select_next_config(config))
		{
			LOG_I("set(ValueUID): all values by UID received, discovery complete");
			finish();
		}

		request_current();
	}

	return stored;
}

bool AMFITRACK_Config::request_current()
{
	// abort discovery if a device disconnects while config is being discovered - previously it would keep retrying forever
	if (_state != CONFIG_DISCOVERY_IDLE && _state != CONFIG_DISCOVERY_DONE &&
		!AMFITRACK_Devices::getInstance().is_device_active(_device_id))
	{
		LOG_W("request_current: device %u no longer active, aborting discovery (state=%d)",
			  _device_id, (int)_state);
		_state = CONFIG_DISCOVERY_IDLE;
		set_waiting_for_reply(false);
		return false;
	}

	if (_waiting_for_reply)
	{
		const uint32_t now = lib_time::get_time_ms();

		if ((_last_request_time != 0) &&
			((now - _last_request_time) < kConfigReplyTimeoutMs))
		{
			return true;
		}

		LOG_W("request_current: no reply within 1s, resending last request (state=%d)", (int)_state);
		set_waiting_for_reply(false);
	}

	switch (_state)
	{
		case CONFIG_DISCOVERY_CATEGORY_COUNT:
			return request_category_count();
		case CONFIG_DISCOVERY_CATEGORY_NAMES:
			return request_category_name();
		case CONFIG_DISCOVERY_VALUE_COUNT:
			return request_value_count();
		case CONFIG_DISCOVERY_NAMES_BY_UID:
			return request_name_by_uid();
		case CONFIG_DISCOVERY_VALUES_BY_UID:
			return request_value_by_uid();
		case CONFIG_DISCOVERY_IDLE:
		case CONFIG_DISCOVERY_DONE:
		default:
			return true;
	}
}

bool AMFITRACK_Config::request_category_count()
{
	LOG_D("request_category_count: device_id=%u", _device_id);
	uint8_t payload = static_cast<uint8_t>(lib_AmfiProt_PayloadID_RequestCategoryCount);
	const bool queued = AmfiProt_API::getInstance().queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, _device_id);
	if (!queued)
	{
		LOG_W("request_category_count: failed to queue frame for device_id=%u", _device_id);
	}
	set_waiting_for_reply(queued);
	return queued;
}

bool AMFITRACK_Config::request_category_name()
{
	DeviceConfig_t config = load_config(_device_id);
	if (_category_index >= config.categoryCount)
	{
		LOG_D("request_category_name: category_index=%u >= categoryCount=%u, advancing",
			  _category_index,
			  config.categoryCount);
		advance_to_value_count(config);
		return request_current();
	}

	LOG_D("request_category_name: requesting category_index=%u", _category_index);

	lib_AmfiProt_ConfigCategoryRequest_t payload = {};
	payload.payloadID = lib_AmfiProt_PayloadID_RequestConfigurationCategory;
	payload.categoryIndex = _category_index;

	const bool queued = AmfiProt_API::getInstance().queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, _device_id);
	if (!queued)
	{
		LOG_W("request_category_name: failed to queue frame for category_index=%u", _category_index);
	}
	set_waiting_for_reply(queued);
	return queued;
}

bool AMFITRACK_Config::request_value_count()
{
	DeviceConfig_t config = load_config(_device_id);
	if (_force_all_config)
	{
		ensure_all_config_category(config);
		_category_index = static_cast<uint8_t>(all_config_storage_index(config));
	}

	if (_category_index >= config.categoryCount)
	{
		LOG_D("request_value_count: category_index=%u >= categoryCount=%u, advancing",
			  _category_index,
			  config.categoryCount);
		advance_to_names_by_uid(config);
		return request_current();
	}

	const uint8_t request_category = _force_all_config ? kAllConfigCategory : config.categories[_category_index].index;
	LOG_D("request_value_count: requesting value count for category=%u (storage_index=%u)",
		  request_category,
		  _category_index);

	lib_AmfiProt_ConfigValueCountRequest_t payload = {};
	payload.payloadID = lib_AmfiProt_PayloadID_RequestConfigurationValueCount;
	payload.category = request_category;

	const bool queued = AmfiProt_API::getInstance().queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, _device_id);
	if (!queued)
	{
		LOG_W("request_value_count: failed to queue frame for category=%u", request_category);
	}
	set_waiting_for_reply(queued);
	return queued;
}

bool AMFITRACK_Config::request_name_by_uid()
{
	DeviceConfig_t config = load_config(_device_id);
	if (!select_next_config(config))
	{
		LOG_D("request_name_by_uid: no more configs, advancing to VALUES_BY_UID");
		advance_to_values_by_uid(config);
		return request_current();
	}

	const uint8_t request_category = _force_all_config ? kAllConfigCategory : config.categories[_category_index].index;
	LOG_D("request_name_by_uid: requesting category=%u, index=%u (storage_index=%u)",
		  request_category,
		  _config_index,
		  _category_index);

	lib_AmfiProt_ConfigNameRequestUID_t payload = {};
	payload.payloadID = lib_AmfiProt_PayloadID_RequestConfigurationNameAndUID;
	payload.category = request_category;
	payload.index = _config_index;

	const bool queued = AmfiProt_API::getInstance().queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, _device_id);
	if (!queued)
	{
		LOG_W("request_name_by_uid: failed to queue frame for category=%u, index=%u",
			  request_category,
			  _config_index);
	}
	set_waiting_for_reply(queued);
	return queued;
}

bool AMFITRACK_Config::request_value_by_uid()
{
	DeviceConfig_t config = load_config(_device_id);
	if (!select_next_config(config))
	{
		LOG_D("request_value_by_uid: no more configs, finishing");
		finish();
		return true;
	}

	const uint32_t uid = config.categories[_category_index].configs[_config_index].uid;
	LOG_D("request_value_by_uid: requesting uid=%u (category=%u, index=%u)",
		  uid,
		  _category_index,
		  _config_index);

	lib_AmfiProt_ConfigValueUIDRequest_t payload = {};
	payload.payloadID = lib_AmfiProt_PayloadID_RequestConfigurationValueUID;
	payload.uid = uid;

	const bool queued = AmfiProt_API::getInstance().queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, _device_id);
	if (!queued)
	{
		LOG_W("request_value_by_uid: failed to queue frame for uid=%u", uid);
	}
	set_waiting_for_reply(queued);
	return queued;
}

void AMFITRACK_Config::set_waiting_for_reply(bool waiting)
{
	_waiting_for_reply = waiting;

	if (waiting)
	{
		_last_request_time = lib_time::get_time_ms();
	}
	else
	{
		_last_request_time = 0;
	}
}

bool AMFITRACK_Config::select_next_config(DeviceConfig_t const &config)
{
	while (_category_index < config.categoryCount)
	{
		if (_config_index < config.categories[_category_index].configCount)
		{
			return true;
		}

		_category_index++;
		_config_index = 0U;
	}

	return false;
}

bool AMFITRACK_Config::is_active(uint8_t device_id, ConfigDiscoveryState_t state) const
{
	return (device_id == _device_id) && (_state == state);
}

void AMFITRACK_Config::advance_to_value_count(DeviceConfig_t const &config)
{
	LOG_I("advance_to_value_count: entering VALUE_COUNT state");
	_state = CONFIG_DISCOVERY_VALUE_COUNT;
	_category_index = _force_all_config ? static_cast<uint8_t>(all_config_storage_index(config)) : 0U;
	_config_index = 0U;
	set_waiting_for_reply(false);
}

void AMFITRACK_Config::advance_to_names_by_uid(DeviceConfig_t const &config)
{
	LOG_I("advance_to_names_by_uid: entering NAMES_BY_UID state");
	_state = CONFIG_DISCOVERY_NAMES_BY_UID;
	_category_index = _force_all_config ? static_cast<uint8_t>(all_config_storage_index(config)) : 0U;
	_config_index = 0U;
	set_waiting_for_reply(false);

	if (!select_next_config(config))
	{
		LOG_I("advance_to_names_by_uid: no configs found, skipping to VALUES_BY_UID");
		advance_to_values_by_uid(config);
	}
}

void AMFITRACK_Config::advance_to_values_by_uid(DeviceConfig_t const &config)
{
	LOG_I("advance_to_values_by_uid: entering VALUES_BY_UID state");
	_state = CONFIG_DISCOVERY_VALUES_BY_UID;
	_category_index = _force_all_config ? static_cast<uint8_t>(all_config_storage_index(config)) : 0U;
	_config_index = 0U;
	set_waiting_for_reply(false);

	if (!select_next_config(config))
	{
		LOG_I("advance_to_values_by_uid: no configs found, finishing");
		finish();
	}
}

static void log_parameter_value(const lib_Generic_Parameter_Value &value)
{
	switch (static_cast<lib_Generic_Parameter_Type_t>(value.type))
	{
		case lib_Generic_Parameter_Type_void:
			// void carries a bool byte (see lib_Generic_Parameter): show it.
			LOG_I("        value=(void/bool)%s", value.b ? "true" : "false");
			break;
		case lib_Generic_Parameter_Type_bool:
			LOG_I("        value=(bool)%s", value.b ? "true" : "false");
			break;
		case lib_Generic_Parameter_Type_char:
			LOG_I("        value=(char)'%c'", value.ch);
			break;
		case lib_Generic_Parameter_Type_s8:
			LOG_I("        value=(int8)%d", (int)value.s8);
			break;
		case lib_Generic_Parameter_Type_u8:
			LOG_I("        value=(uint8)%u", (unsigned)value.u8);
			break;
		case lib_Generic_Parameter_Type_s16LE:
		case lib_Generic_Parameter_Type_s16BE:
			LOG_I("        value=(int16%s)%d",
				  value.type == lib_Generic_Parameter_Type_s16LE ? "LE" : "BE",
				  (int)value.s16);
			break;
		case lib_Generic_Parameter_Type_u16LE:
		case lib_Generic_Parameter_Type_u16BE:
			LOG_I("        value=(uint16%s)%u",
				  value.type == lib_Generic_Parameter_Type_u16LE ? "LE" : "BE",
				  (unsigned)value.u16);
			break;
		case lib_Generic_Parameter_Type_s32LE:
		case lib_Generic_Parameter_Type_s32BE:
			LOG_I("        value=(int32%s)%ld",
				  value.type == lib_Generic_Parameter_Type_s32LE ? "LE" : "BE",
				  (long)value.s32);
			break;
		case lib_Generic_Parameter_Type_u32LE:
		case lib_Generic_Parameter_Type_u32BE:
			LOG_I("        value=(uint32%s)%lu",
				  value.type == lib_Generic_Parameter_Type_u32LE ? "LE" : "BE",
				  (unsigned long)value.u32);
			break;
		case lib_Generic_Parameter_Type_s64LE:
		case lib_Generic_Parameter_Type_s64BE:
			LOG_I("        value=(int64%s)%lld",
				  value.type == lib_Generic_Parameter_Type_s64LE ? "LE" : "BE",
				  (long long)value.s64);
			break;
		case lib_Generic_Parameter_Type_u64LE:
		case lib_Generic_Parameter_Type_u64BE:
			LOG_I("        value=(uint64%s)%llu",
				  value.type == lib_Generic_Parameter_Type_u64LE ? "LE" : "BE",
				  (unsigned long long)value.u64);
			break;
		case lib_Generic_Parameter_Type_f32LE:
		case lib_Generic_Parameter_Type_f32BE:
			LOG_I("        value=(float%s)%f",
				  value.type == lib_Generic_Parameter_Type_f32LE ? "LE" : "BE",
				  (double)value.f32);
			break;
		case lib_Generic_Parameter_Type_f64LE:
		case lib_Generic_Parameter_Type_f64BE:
			LOG_I("        value=(double%s)%f",
				  value.type == lib_Generic_Parameter_Type_f64LE ? "LE" : "BE",
				  value.f64);
			break;
		case lib_Generic_Parameter_Type_ProcedureCall:
			LOG_I("        value=(ProcedureCall)");
			break;
		default:
			LOG_W("        value=(unknown type=%u)", value.type);
			break;
	}
}

void AMFITRACK_Config::print_config(uint8_t device_id)
{
	DeviceConfig_t config = load_config(device_id);

	LOG_I("==============================");
	LOG_I("Config dump for device_id=%u", device_id);
	LOG_I("  categoryCount=%u", config.categoryCount);

	bool printed_category = false;
	for (std::size_t category_index = 0U; category_index < config.categoryCount; category_index++)
	{
		const CategoryEntry_t &category = config.categories[category_index];
		if ((category.configCount == 0U) && (category.name[0] == '\0'))
		{
			continue;
		}

		printed_category = true;
		LOG_I("  [Category %u] \"%s\" (%u config(s))",
			  category.index,
			  category.name,
			  category.configCount);

		for (std::size_t config_index = 0U; config_index < category.configCount; config_index++)
		{
			const ConfigEntry_t &entry = category.configs[config_index];
			LOG_I("    [%u] \"%s\" (uid=%u)", (unsigned)config_index, entry.name, entry.uid);
			log_parameter_value(entry.config);
		}
	}

	if (!printed_category)
	{
		LOG_I("  (no categories populated)");
	}

	LOG_I("==============================");
}

void AMFITRACK_Config::finish()
{
	LOG_I("finish: config discovery complete for device_id=%u", _device_id);
	_state = CONFIG_DISCOVERY_DONE;
	_category_index = 0U;
	_config_index = 0U;
	_force_all_config = false;
	set_waiting_for_reply(false);

	print_config(_device_id);
}
