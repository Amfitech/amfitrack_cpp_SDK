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
#include "Amfitrack_resetinfo.h"

#include "Amfitrack_Devices.h"
#include "Amfitrack_Sensor.h"
#include "Amfitrack_Source.h"
#include "lib_AmfiProt_API.hpp"
#include "lib_log.h"
#include "lib_time.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------
#define CFSR_MMARVALID 0x00000080u
#define CFSR_BFARVALID 0x00008000u

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

namespace
{
constexpr uint32_t kResetInfoReplyTimeoutMs = 1000;

// A device that never answers must not spin forever: this is a diagnostic path,
// unlike config discovery which retries indefinitely.
constexpr uint8_t kResetInfoMaxAttempts = 3;

constexpr std::size_t kChunkHeaderLength = 4U;

// The three helpers below enumerate the fields by hand.
static_assert(kResetInfoFieldCount == 3U, "field <-> state mapping helpers need updating");

ResetInfoState_t state_for_field_index(std::size_t index)
{
	switch (index)
	{
		case 0U:
			return RESET_INFO_FILE;
		case 1U:
			return RESET_INFO_FUNC;
		default:
			return RESET_INFO_EXPR;
	}
}

bool field_index_for_state(ResetInfoState_t state, std::size_t *index)
{
	switch (state)
	{
		case RESET_INFO_FILE:
			*index = 0U;
			return true;
		case RESET_INFO_FUNC:
			*index = 1U;
			return true;
		case RESET_INFO_EXPR:
			*index = 2U;
			return true;
		default:
			return false;
	}
}

uint8_t wire_field_for_state(ResetInfoState_t state)
{
	switch (state)
	{
		case RESET_INFO_FILE:
			return static_cast<uint8_t>(lib_AmfiProt_ResetInfoField_File);
		case RESET_INFO_FUNC:
			return static_cast<uint8_t>(lib_AmfiProt_ResetInfoField_Func);
		default:
			return static_cast<uint8_t>(lib_AmfiProt_ResetInfoField_Expr);
	}
}

// MMFSR (bits 0-7), BFSR (8-15) and UFSR (16-31) packed into one word, per
// ARMv7-M. Fixed by the architecture, so the table cannot go stale.
struct
{
	uint32_t mask;
	char const *name;
} const kCfsrFlags[] = {
	{0x00000001u, "IACCVIOL"},
	{0x00000002u, "DACCVIOL"},
	{0x00000008u, "MUNSTKERR"},
	{0x00000010u, "MSTKERR"},
	{0x00000020u, "MLSPERR"},
	{0x00000080u, "MMARVALID"},
	{0x00000100u, "IBUSERR"},
	{0x00000200u, "PRECISERR"},
	{0x00000400u, "IMPRECISERR"},
	{0x00000800u, "UNSTKERR"},
	{0x00001000u, "STKERR"},
	{0x00002000u, "LSPERR"},
	{0x00008000u, "BFARVALID"},
	{0x00010000u, "UNDEFINSTR"},
	{0x00020000u, "INVSTATE"},
	{0x00040000u, "INVPC"},
	{0x00080000u, "NOCP"},
	{0x01000000u, "UNALIGNED"},
	{0x02000000u, "DIVBYZERO"},
};

char *field_buffer(ResetInfo_t &info, std::size_t index)
{
	switch (index)
	{
		case 0U:
			return info.file;
		case 1U:
			return info.func;
		default:
			return info.expr;
	}
}
} // namespace

std::size_t resetinfo_chunk_text_length(std::size_t payload_length)
{
	if (payload_length < kChunkHeaderLength)
	{
		return 0U;
	}

	return payload_length - kChunkHeaderLength;
}

bool resetinfo_chunk_is_expected(uint8_t chunk_index, uint8_t chunk_count, uint8_t expected_index)
{
	if ((chunk_count == 0U) || (chunk_count > AMFITRACK_RESET_INFO_MAX_CHUNKS))
	{
		return false;
	}

	if (chunk_index >= chunk_count)
	{
		return false;
	}

	return chunk_index == expected_index;
}

std::size_t resetinfo_append_text(char *dest, std::size_t dest_size, std::size_t offset, char const *text, std::size_t text_length)
{
	if ((dest == nullptr) || (dest_size == 0U))
	{
		return 0U;
	}

	if (offset >= (dest_size - 1U))
	{
		dest[dest_size - 1U] = '\0';
		return dest_size - 1U;
	}

	const std::size_t room = (dest_size - 1U) - offset;
	const std::size_t copy_length = (text == nullptr) ? 0U : std::min(text_length, room);

	for (std::size_t i = 0U; i < copy_length; i++)
	{
		const char c = text[i];
		dest[offset + i] = ((c >= 0x20) && (c < 0x7F)) ? c : '.';
	}

	dest[offset + copy_length] = '\0';
	return offset + copy_length;
}

// Record 0 always answers, even with nothing logged - it comes back with
// recordType None. A device that never faulted reports available == 0, so
// clamping straight to it would walk nothing at all.
std::size_t resetinfo_records_to_read(std::size_t requested, std::size_t available)
{
	const std::size_t wanted = std::max<std::size_t>(requested, 1U);
	const std::size_t reachable = std::min(std::max<std::size_t>(available, 1U), kResetInfoMaxRecords);

	return std::min(wanted, reachable);
}

// Mirrors drv_BootDiagn_reset_reason_t in the device firmware. Unlike
// recordType there is no wire enum to derive this from, so the table is a hand
// copy and must be kept in step with the firmware header.
char const *resetinfo_reset_reason_name(uint8_t reason)
{
	static char const *const names[] = {
		"UNKNOWN",
		"LOW_POWER",
		"WINDOW_WATCHDOG",
		"INDEPENDENT_WATCHDOG",
		"SOFTWARE",
		"POR_PDR",
		"PIN",
		"BROWNOUT",
	};

	return (reason < (sizeof(names) / sizeof(names[0]))) ? names[reason] : "INVALID";
}

char const *resetinfo_record_type_name(uint8_t record_type)
{
	switch (record_type)
	{
		case lib_AmfiProt_ResetInfoRecord_None:
			return "none";
		case lib_AmfiProt_ResetInfoRecord_HardFault:
			return "hardfault";
		case lib_AmfiProt_ResetInfoRecord_Assert:
			return "assert";
		case lib_AmfiProt_ResetInfoRecord_Reset:
			return "reset";
		default:
			return "unknown";
	}
}

std::size_t resetinfo_cfsr_to_string(uint32_t cfsr, char *out, std::size_t size)
{
	if ((out == nullptr) || (size == 0U))
	{
		return 0U;
	}

	std::size_t pos = 0U;
	bool any = false;
	out[0] = '\0';

	for (std::size_t i = 0U; i < (sizeof(kCfsrFlags) / sizeof(kCfsrFlags[0])); i++)
	{
		if ((cfsr & kCfsrFlags[i].mask) == 0U)
		{
			continue;
		}

		any = true;

		// Measured before writing: letting snprintf truncate would leave a
		// dangling " | " separator with no name after it.
		const std::size_t separator = (pos == 0U) ? 0U : 3U;
		const std::size_t needed = separator + std::strlen(kCfsrFlags[i].name);

		if ((pos + needed) >= size)
		{
			break;
		}

		const int written = std::snprintf(out + pos, size - pos, (pos == 0U) ? "%s" : " | %s", kCfsrFlags[i].name);

		if (written < 0)
		{
			break;
		}

		pos += static_cast<std::size_t>(written);
	}

	// An empty result with bits set means the buffer was too small for even the
	// first name; reporting "none" there would misread the fault.
	if (!any)
	{
		const int written = std::snprintf(out, size, "none");
		return (written < 0) ? 0U : std::strlen(out);
	}

	return pos;
}

// One physical register serves as both MMFAR and BFAR, so a stale address reads
// back on a fault that set neither valid bit.
std::size_t resetinfo_fault_address_to_string(uint32_t address, uint32_t cfsr, char *out, std::size_t size)
{
	if ((out == nullptr) || (size == 0U))
	{
		return 0U;
	}

	int written;

	if ((cfsr & CFSR_MMARVALID) != 0U)
	{
		written = std::snprintf(out, size, "0x%08X (MMFAR)", address);
	}
	else if ((cfsr & CFSR_BFARVALID) != 0U)
	{
		written = std::snprintf(out, size, "0x%08X (BFAR)", address);
	}
	else
	{
		written = std::snprintf(out, size, "invalid");
	}

	return (written < 0) ? 0U : std::strlen(out);
}

// pc and stringAddr only resolve against the image that wrote the record, so the
// build is part of reading one.
std::size_t resetinfo_firmware_to_string(uint32_t fw_mmp, uint32_t fw_build, char *out, std::size_t size)
{
	if ((out == nullptr) || (size == 0U))
	{
		return 0U;
	}

	const int written = std::snprintf(out, size, "%u.%u.%u build %u",
									  (unsigned)((fw_mmp >> 16) & 0xFFu),
									  (unsigned)((fw_mmp >> 8) & 0xFFu),
									  (unsigned)(fw_mmp & 0xFFu),
									  (unsigned)fw_build);

	return (written < 0) ? 0U : std::strlen(out);
}

AMFITRACK_ResetInfo &AMFITRACK_ResetInfo::getInstance()
{
	static AMFITRACK_ResetInfo instance;
	return instance;
}

bool AMFITRACK_ResetInfo::start(uint8_t device_id, uint8_t record_count)
{
	if (!AMFITRACK_Devices::is_valid_device_id(device_id))
	{
		LOG_E("start reset info: invalid device_id=%u", device_id);
		return false;
	}

	if (!AMFITRACK_Devices::getInstance().is_device_active(device_id))
	{
		LOG_W("start reset info: device not active=%u", device_id);
		return false;
	}

#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	// 0 and 1 both mean the newest record: index 0 always answers, so there is no
	// "fetch nothing" case to express.
	_requested = (record_count == 0U) ? 1U : record_count;

	LOG_I("start: requesting %u reset info record(s) for device_id=%u", _requested, device_id);

	_device_id = device_id;
	_state = RESET_INFO_SUMMARY;
	_record_index = 0U;
	_record_count = 0U;
	_chunk_index = 0U;
	std::memset(_chunk_count, 0, sizeof(_chunk_count));
	std::memset(_length, 0, sizeof(_length));
	_info = ResetInfo_t{};
	_records.clear();
	_attempts = 0U;

	return request_current();
}

void AMFITRACK_ResetInfo::run()
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	request_current();
}

ResetInfoState_t AMFITRACK_ResetInfo::state(uint8_t device_id) const
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (device_id != _device_id)
	{
		return RESET_INFO_IDLE;
	}

	return _state;
}

bool AMFITRACK_ResetInfo::get(uint8_t device_id, ResetInfoLog_t *out) const
{
	if (out == nullptr)
	{
		return false;
	}

	AMFITRACK_Sensor sensor;
	if (AMFITRACK_Devices::getInstance().get_sensor_by_id(device_id, &sensor))
	{
		*out = sensor.resetInfo;
		return !out->records.empty();
	}

	AMFITRACK_Source source;
	if (AMFITRACK_Devices::getInstance().get_source_by_id(device_id, &source))
	{
		*out = source.resetInfo;
		return !out->records.empty();
	}

	return false;
}

bool AMFITRACK_ResetInfo::get(uint8_t device_id, uint8_t record_index, ResetInfo_t *out) const
{
	if (out == nullptr)
	{
		return false;
	}

	ResetInfoLog_t log;
	if (!get(device_id, &log))
	{
		return false;
	}

	// A record the walk gave up on leaves a hole, so the index is searched for
	// rather than used as a subscript.
	for (std::size_t i = 0U; i < log.records.size(); i++)
	{
		if (log.records[i].recordIndex == record_index)
		{
			*out = log.records[i];
			return out->valid;
		}
	}

	return false;
}

bool AMFITRACK_ResetInfo::set(uint8_t device_id, lib_AmfiProt_ResetInfoSummary_t const &summary)
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (!is_active(device_id, RESET_INFO_SUMMARY))
	{
		return false;
	}

	// A reply that crossed with a resend answers an index the walk has already
	// moved past; taking it would file the wrong record under the current index.
	if (summary.recordIndex != _record_index)
	{
		LOG_W("set summary: device %u answered record %u while fetching record %u",
			  device_id, summary.recordIndex, _record_index);
		return false;
	}

	_info = ResetInfo_t{};
	_info.recordIndex = summary.recordIndex;
	_info.recordCount = summary.recordCount;
	_info.resetReason = summary.resetReason;
	_info.recordType = summary.recordType;
	_info.cfsr = summary.cfsr;
	_info.xFAR = summary.xFAR;
	_info.pc = summary.pc;
	_info.lr = summary.lr;
	_info.psr = summary.psr;
	_info.assertLine = summary.assertLine;
	_info.fw_mmp = summary.fw_mmp;
	_info.fw_build = summary.fw_build;
	_info.valid = true;

	for (std::size_t i = 0U; i < kResetInfoFieldCount; i++)
	{
		_info.stringAddr[i] = summary.stringAddr[i];
		_chunk_count[i] = summary.chunkCount[i];
		_length[i] = 0U;

		if (_chunk_count[i] > AMFITRACK_RESET_INFO_MAX_CHUNKS)
		{
			LOG_W("set summary: device %u reported chunkCount=%u for field %u, clamping",
				  device_id, _chunk_count[i], (unsigned)(i + 1U));
			_chunk_count[i] = AMFITRACK_RESET_INFO_MAX_CHUNKS;
		}
	}

	// The count is read once, from record 0: a later record answers with whatever
	// the log held when it was written, which need not be the current total.
	if (_record_index == 0U)
	{
		_record_count = summary.recordCount;
	}

	LOG_I("set summary: device_id=%u, record=%u/%u, reason=%u, type=%u",
		  device_id, _info.recordIndex, _info.recordCount, _info.resetReason, _info.recordType);

	advance_after_summary();
	request_current();
	return true;
}

bool AMFITRACK_ResetInfo::set(uint8_t device_id, lib_AmfiProt_ResetInfoChunk_t const &chunk, uint8_t payload_length)
{
#ifdef USE_THREAD_BASED
	const std::lock_guard<std::mutex> lock(_mutex);
#endif

	std::size_t index = 0U;
	if ((device_id != _device_id) || !field_index_for_state(_state, &index))
	{
		return false;
	}

	if (chunk.field != wire_field_for_state(_state))
	{
		LOG_W("set chunk: device %u answered field %u while fetching field %u",
			  device_id, chunk.field, wire_field_for_state(_state));
		return false;
	}

	// A chunk carries no recordIndex, so the field and index checks are all that
	// separate it from a late reply belonging to the previous record.
	if (!resetinfo_chunk_is_expected(chunk.chunkIndex, chunk.chunkCount, _chunk_index))
	{
		LOG_W("set chunk: device %u sent index=%u count=%u, expected index=%u",
			  device_id, chunk.chunkIndex, chunk.chunkCount, _chunk_index);
		return false;
	}

	const std::size_t text_length = resetinfo_chunk_text_length(payload_length);
	char *dest = field_buffer(_info, index);

	_length[index] = resetinfo_append_text(dest, AMFITRACK_RESET_INFO_STRING_LENGTH, _length[index], chunk.text, text_length);

	_attempts = 0U;
	_chunk_index++;

	// chunkCount is repeated on every chunk, so a host that lost the summary can
	// still tell when the field is complete.
	if (_chunk_index >= chunk.chunkCount)
	{
		advance_field();
	}

	request_current();
	return true;
}

void AMFITRACK_ResetInfo::print_reset_info(uint8_t device_id) const
{
	ResetInfoLog_t log;

	if (!get(device_id, &log))
	{
		LOG_W("print_reset_info: no reset info stored for device_id=%u", device_id);
		return;
	}

	LOG_I("=== ResetInfo: device %u, %u record(s) logged, %u read ===",
		  device_id, log.count, (unsigned)log.records.size());

	for (std::size_t r = 0U; r < log.records.size(); r++)
	{
		ResetInfo_t const &info = log.records[r];
		char firmware[48];

		resetinfo_firmware_to_string(info.fw_mmp, info.fw_build, firmware, sizeof(firmware));

		LOG_I("--- record %u (%s) --------------------------------",
			  info.recordIndex, (info.recordIndex == 0U) ? "newest" : "older");
		LOG_I("  record       : %s", resetinfo_record_type_name(info.recordType));
		LOG_I("  written by   : %s", firmware);

		switch (info.recordType)
		{
			case lib_AmfiProt_ResetInfoRecord_Assert:
				LOG_I("  assert line  : %u", info.assertLine);
				LOG_I("  PC           : 0x%08X", info.pc);

				// The device only serves strings for records its own build wrote;
				// for anything older the pointers are all the host gets.
				if (info.file[0] != '\0')
				{
					LOG_I("  assert       : %s:%u", info.file, info.assertLine);
					LOG_I("  in function  : %s()", info.func);
					LOG_I("  expression   : %s", info.expr);
				}
				else
				{
					LOG_I("  strings      : not served - record predates the running build");
					LOG_I("  file/func/expr: 0x%08X / 0x%08X / 0x%08X (objdump the %s image)",
						  info.stringAddr[0], info.stringAddr[1], info.stringAddr[2], firmware);
				}
				break;

			case lib_AmfiProt_ResetInfoRecord_HardFault:
			{
				char flags[256];
				char xfar[32];

				resetinfo_cfsr_to_string(info.cfsr, flags, sizeof(flags));
				resetinfo_fault_address_to_string(info.xFAR, info.cfsr, xfar, sizeof(xfar));

				const uint32_t exception = info.psr & 0x1FFu;

				LOG_I("  fault        : %s", flags);
				LOG_I("  CFSR         : 0x%08X", info.cfsr);
				LOG_I("  PC / LR      : 0x%08X / 0x%08X", info.pc, info.lr);
				LOG_I("  xFAR         : %s", xfar);
				LOG_I("  PSR          : 0x%08X (%s, T-bit %u)",
					  info.psr,
					  (exception == 0u) ? "thread mode" : "in exception",
					  (unsigned)((info.psr >> 24) & 1u));
				break;
			}

			case lib_AmfiProt_ResetInfoRecord_Reset:
				// Nothing ran to capture fault context - the reason is the record.
				LOG_I("  reset reason : %s (%u)", resetinfo_reset_reason_name(info.resetReason), info.resetReason);
				LOG_I("  (device did not cause this reset; identical repeats stop being logged after 8)");
				break;

			default:
				LOG_I("  boot reason  : %s (%u)", resetinfo_reset_reason_name(info.resetReason), info.resetReason);
				LOG_I("  (no fault record retained - the reason above is this boot's, not a logged one)");
				break;
		}
	}

	LOG_I("===================================================");
}

bool AMFITRACK_ResetInfo::request_current()
{
	if ((_state == RESET_INFO_IDLE) || (_state == RESET_INFO_DONE) ||
		(_state == RESET_INFO_FAILED))
	{
		return true;
	}

	if (!AMFITRACK_Devices::getInstance().is_device_active(_device_id))
	{
		LOG_W("request_current: device %u no longer active, aborting reset info (state=%d)",
			  _device_id, (int)_state);
		fail();
		return false;
	}

	// _attempts covers both a lost reply and a request that never reached the TX
	// FIFO, so neither can be resent at the loop rate without tripping the cap.
	if (_attempts > 0U)
	{
		const uint32_t now = lib_time::get_time_ms();

		if ((now - _last_attempt_time) < kResetInfoReplyTimeoutMs)
		{
			return true;
		}

		if (_attempts >= kResetInfoMaxAttempts)
		{
			LOG_W("request_current: device %u gave no reply for record %u after %u attempts (state=%d)",
				  _device_id, _record_index, _attempts, (int)_state);
			give_up_on_record();
			return false;
		}

		LOG_W("request_current: no reply within %ums, resending (record=%u, state=%d, attempt=%u)",
			  kResetInfoReplyTimeoutMs, _record_index, (int)_state, _attempts);
	}

	if (_state == RESET_INFO_SUMMARY)
	{
		return request_summary();
	}

	return request_chunk();
}

bool AMFITRACK_ResetInfo::request_summary()
{
	LOG_D("request_summary: device_id=%u, record=%u", _device_id, _record_index);

	lib_AmfiProt_ResetInfoRequest_t payload = {};
	payload.payloadID = static_cast<uint8_t>(lib_AmfiProt_PayloadID_RequestResetInfo);
	payload.field = static_cast<uint8_t>(lib_AmfiProt_ResetInfoField_Summary);
	payload.chunkIndex = 0U;
	payload.recordIndex = _record_index;

	const bool queued = AmfiProt_API::getInstance().queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, _device_id);
	if (!queued)
	{
		LOG_W("request_summary: failed to queue frame for device_id=%u", _device_id);
	}

	note_attempt();
	return queued;
}

bool AMFITRACK_ResetInfo::request_chunk()
{
	LOG_D("request_chunk: device_id=%u, record=%u, field=%u, index=%u",
		  _device_id, _record_index, wire_field_for_state(_state), _chunk_index);

	lib_AmfiProt_ResetInfoRequest_t payload = {};
	payload.payloadID = static_cast<uint8_t>(lib_AmfiProt_PayloadID_RequestResetInfo);
	payload.field = wire_field_for_state(_state);
	payload.chunkIndex = _chunk_index;
	payload.recordIndex = _record_index;

	const bool queued = AmfiProt_API::getInstance().queue_frame(&payload, sizeof(payload), libAmfiProt_PayloadType_Common, lib_AmfiProt_packetType_NoAck, _device_id);
	if (!queued)
	{
		LOG_W("request_chunk: failed to queue frame for device_id=%u, field=%u", _device_id, payload.field);
	}

	note_attempt();
	return queued;
}

void AMFITRACK_ResetInfo::note_attempt()
{
	// Counted even when queue_frame() failed, so a full TX FIFO is rate-limited
	// by the same timeout and still trips the attempt cap.
	_attempts++;
	_last_attempt_time = lib_time::get_time_ms();
}

bool AMFITRACK_ResetInfo::is_active(uint8_t device_id, ResetInfoState_t expected) const
{
	return (device_id == _device_id) && (_state == expected);
}

void AMFITRACK_ResetInfo::advance_after_summary()
{
	// A hard fault carries no strings, and neither does a normal reboot:
	// chunkCount is all zero and the record is one round trip.
	_state = RESET_INFO_FILE;
	_chunk_index = 0U;
	_attempts = 0U;

	std::size_t index = 0U;
	if (field_index_for_state(_state, &index) && (_chunk_count[index] == 0U))
	{
		advance_field();
	}
}

void AMFITRACK_ResetInfo::advance_field()
{
	std::size_t index = 0U;
	if (!field_index_for_state(_state, &index))
	{
		finish_record();
		return;
	}

	for (std::size_t next = index + 1U; next < kResetInfoFieldCount; next++)
	{
		if (_chunk_count[next] > 0U)
		{
			_state = state_for_field_index(next);
			_chunk_index = 0U;
			_attempts = 0U;
			return;
		}
	}

	finish_record();
}

std::size_t AMFITRACK_ResetInfo::records_to_read() const
{
	return resetinfo_records_to_read(_requested, _record_count);
}

void AMFITRACK_ResetInfo::finish_record()
{
	if (_info.valid)
	{
		_records.push_back(_info);
	}

	const std::size_t next = static_cast<std::size_t>(_record_index) + 1U;

	if (next >= records_to_read())
	{
		finish();
		return;
	}

	_record_index = static_cast<uint8_t>(next);
	_state = RESET_INFO_SUMMARY;
	_chunk_index = 0U;
	std::memset(_chunk_count, 0, sizeof(_chunk_count));
	std::memset(_length, 0, sizeof(_length));
	_info = ResetInfo_t{};
	_attempts = 0U;
}

// A record that will not read is not the end of the log: it was counted as
// intact at startup and has since gone marginal, so the walk continues at the
// next index. Only record 0 failing ends the exchange - without its recordCount
// there is nothing to walk, and that is also how firmware without ResetInfo
// support presents itself, since an error reply cannot be attributed.
void AMFITRACK_ResetInfo::give_up_on_record()
{
	if ((_state == RESET_INFO_SUMMARY) && (_record_index == 0U))
	{
		fail();
		return;
	}

	if (_state == RESET_INFO_SUMMARY)
	{
		LOG_W("give_up_on_record: device %u record %u unreadable, continuing at %u",
			  _device_id, _record_index, _record_index + 1U);
		_info = ResetInfo_t{};
	}
	else
	{
		// The summary is the diagnosis; a string that stalled leaves the record
		// worth keeping, truncated.
		LOG_W("give_up_on_record: device %u record %u strings incomplete, keeping the summary",
			  _device_id, _record_index);
	}

	finish_record();
}

void AMFITRACK_ResetInfo::store()
{
	ResetInfoLog_t log;
	log.count = _record_count;
	log.records = _records;

	if (!AMFITRACK_Devices::getInstance().set(_device_id, AMFITRACK_Devices::deviceType_t::Both, log))
	{
		LOG_E("store: failed to store reset info for device_id=%u", _device_id);
	}
}

void AMFITRACK_ResetInfo::finish()
{
	_state = RESET_INFO_DONE;
	store();

	LOG_I("finish: reset info complete for device_id=%u, %u record(s) read of %u logged",
		  _device_id, (unsigned)_records.size(), _record_count);
}

void AMFITRACK_ResetInfo::fail()
{
	_state = RESET_INFO_FAILED;

	// Records that already arrived are worth keeping even though the walk did not
	// get through the rest.
	if (!_records.empty())
	{
		store();
	}
}
