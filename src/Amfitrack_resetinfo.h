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
#include "AmfitrackDeviceTypes.h"
#include "lib_AmfiProt.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#ifdef USE_THREAD_BASED
#include <mutex>
#endif

//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------

// Number of retained strings (file, func, expr). Distinct from
// AMFITRACK_RESET_INFO_MAX_CHUNKS, which bounds the chunks *within* one field:
// the two are equal today only by coincidence. Derived from the wire enum so
// the per-field arrays cannot drift from the summary's chunkCount.
constexpr std::size_t kResetInfoFieldCount = static_cast<std::size_t>(lib_AmfiProt_ResetInfoField_Last) - 1U;
static_assert(kResetInfoFieldCount == sizeof_member(lib_AmfiProt_ResetInfoSummary_t, chunkCount),
			  "ResetInfo field count disagrees with the summary's chunkCount array");
static_assert(kResetInfoFieldCount == AMFITRACK_RESET_INFO_FIELDS,
			  "ResetInfo field count disagrees with ResetInfo_t::stringAddr");

// A field the host cannot fully reassemble would be silently truncated.
static_assert(AMFITRACK_RESET_INFO_MAX_CHUNKS * sizeof_member(lib_AmfiProt_ResetInfoChunk_t, text) >=
				  (AMFITRACK_RESET_INFO_STRING_LENGTH - 1U),
			  "AMFITRACK_RESET_INFO_MAX_CHUNKS too small for AMFITRACK_RESET_INFO_STRING_LENGTH");

// recordIndex and recordCount are both uint8, so only the 255 newest records in
// the device's flash log are reachable however many it physically holds.
constexpr std::size_t kResetInfoMaxRecords = 255U;

//-----------------------------------------------------------------------------
// Section: Typedef
//-----------------------------------------------------------------------------
typedef enum
{
	RESET_INFO_IDLE,
	RESET_INFO_SUMMARY,
	RESET_INFO_FILE,
	RESET_INFO_FUNC,
	RESET_INFO_EXPR,
	RESET_INFO_DONE,
	RESET_INFO_FAILED, /**< Record 0 never answered, or the device went away mid-walk */
} ResetInfoState_t;

//-----------------------------------------------------------------------------
// Section: Macro
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Section: Function prototypes
//-----------------------------------------------------------------------------

// Free helpers so the reassembly can be exercised without a device attached.

// Number of text bytes in a chunk reply of this payload length; 0 if malformed.
std::size_t resetinfo_chunk_text_length(std::size_t payload_length);

// Rejects a chunk that does not belong where the exchange currently is. The
// device holds no state between requests, so a stale or duplicated reply is
// entirely possible.
bool resetinfo_chunk_is_expected(uint8_t chunk_index, uint8_t chunk_count, uint8_t expected_index);

// Appends text at offset, sanitizing to printable ASCII and always leaving dest
// NUL terminated. Returns the new offset, clamped to dest_size - 1: the device
// bounds strings at 128 chars but a retained record may hold a stale pointer, so
// the text is untrusted.
std::size_t resetinfo_append_text(char *dest, std::size_t dest_size, std::size_t offset, char const *text, std::size_t text_length);

// Records to walk for a caller that asked for `requested` of the `available` the
// device reports. Index 0 always answers, so this is never 0: a device that has
// never faulted reports available == 0 and still has a record 0 to read.
std::size_t resetinfo_records_to_read(std::size_t requested, std::size_t available);

// Name of a ResetInfo_t::resetReason value; "INVALID" when out of range.
char const *resetinfo_reset_reason_name(uint8_t reason);

// Name of a lib_AmfiProt_ResetInfoRecord_t value; "unknown" when out of range.
char const *resetinfo_record_type_name(uint8_t record_type);

// Renders the set CFSR bits as "IACCVIOL | MMARVALID", or "none" when the word
// is clear. Always NUL terminates; returns the length written, which is short of
// the full list if out was too small.
std::size_t resetinfo_cfsr_to_string(uint32_t cfsr, char *out, std::size_t size);

// Renders xFAR as "0x20001000 (BFAR)", or "invalid" when neither valid bit is
// set. Always NUL terminates; returns the length written.
std::size_t resetinfo_fault_address_to_string(uint32_t address, uint32_t cfsr, char *out, std::size_t size);

// Renders the build that wrote a record as "1.2.3 build 456" - the .elf needed to
// resolve its pc and stringAddr. Always NUL terminates; returns the length written.
std::size_t resetinfo_firmware_to_string(uint32_t fw_mmp, uint32_t fw_build, char *out, std::size_t size);

//-----------------------------------------------------------------------------
// Section: Class
//-----------------------------------------------------------------------------

class AMFITRACK_ResetInfo
{

  public:
	static AMFITRACK_ResetInfo &getInstance();

	// `record_count` is how many of the newest records to walk: 0 or 1 is the
	// newest only, 3 the newest three. Clamped to what the device reports.
	bool start(uint8_t device_id, uint8_t record_count = 0U);
	void run();
	ResetInfoState_t state(uint8_t device_id) const;

	bool get(uint8_t device_id, ResetInfoLog_t *out) const;
	bool get(uint8_t device_id, uint8_t record_index, ResetInfo_t *out) const;

	bool set(uint8_t device_id, lib_AmfiProt_ResetInfoSummary_t const &summary);
	bool set(uint8_t device_id, lib_AmfiProt_ResetInfoChunk_t const &chunk, uint8_t payload_length);

	void print_reset_info(uint8_t device_id) const;

  private:
	AMFITRACK_ResetInfo() = default;
	~AMFITRACK_ResetInfo() = default;

	AMFITRACK_ResetInfo(AMFITRACK_ResetInfo const &) = delete;
	AMFITRACK_ResetInfo &operator=(AMFITRACK_ResetInfo const &) = delete;

	bool request_current();
	bool request_summary();
	bool request_chunk();

	void note_attempt();
	bool is_active(uint8_t device_id, ResetInfoState_t expected) const;
	void advance_after_summary();
	void advance_field();
	std::size_t records_to_read() const;
	void finish_record();
	void give_up_on_record();
	void store();
	void finish();
	void fail();

#ifdef USE_THREAD_BASED
	mutable std::mutex _mutex;
#endif

	uint8_t _device_id = 0U;
	ResetInfoState_t _state = RESET_INFO_IDLE;
	uint8_t _record_index = 0U;
	uint8_t _record_count = 0U;  /**< Reported by the device; only known once record 0 answered */
	uint8_t _requested = 1U;     /**< What the caller asked for, before clamping to _record_count */
	uint8_t _chunk_index = 0U;
	uint8_t _chunk_count[kResetInfoFieldCount] = {0U};
	std::size_t _length[kResetInfoFieldCount] = {0U};
	ResetInfo_t _info = {};      /**< The record being assembled */
	std::vector<ResetInfo_t> _records;
	uint32_t _last_attempt_time = 0;
	uint8_t _attempts = 0U;
};
