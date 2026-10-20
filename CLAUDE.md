# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

This is a CMake static library (`Amfitrack_API`). It is designed to be consumed as a subdirectory by a parent project, not built standalone. CMake presets are provided for convenience:

**Windows (MSVC + Ninja):**
```sh
cmake --preset x64-debug
cmake --build out/build/x64-debug
```

**Linux (GCC + Ninja):**
```sh
cmake --preset ninja-gcc
cmake --build build
```

Available presets: `x64-debug`, `x64-release`, `x86-debug`, `x86-release`, `ninja-gcc`.

There are currently no test targets in this repository.

## CMake Options

| Option | Default | Effect |
|--------|---------|--------|
| `AMFITRACK_USE_USB` | ON | Compiles HIDAPI and adds `USE_USB` define |
| `AMFITRACK_USE_THREAD_BASED` | ON | Adds `USE_THREAD_BASED` define enabling mutexes and background thread |

Code throughout the library is guarded with `#ifdef USE_USB` and `#ifdef USE_THREAD_BASED`.

## Architecture

### Public API
`AMFITRACK` ([Amfitrack.h](Amfitrack.h)) is the single entry point — a singleton. The typical call sequence is:
1. `AMFITRACK::getInstance().init()` — initializes the USB monitor and protocol layer
2. `AMFITRACK::getInstance().start_task()` — starts background thread (requires `USE_THREAD_BASED`)
3. Alternatively, call `run()` in a polling loop instead of using a thread
4. Read data via `get_sensor(device_id, &sensor)` / `get_source(device_id, &source)`

Below the `// Old function will be deprecated` banner, [Amfitrack.h](Amfitrack.h) carries an older API (`initialize_amfitrack()`, `start_amfitrack_task()`, `getDevicePose()`, `getSensorMeasurements()`, …) that overlaps the one above it. Extend the new API; leave the deprecated block alone.

### Singleton Layers
All major components are singletons with deleted copy/assignment operators:

- **`AMFITRACK`** — public façade; delegates to all other singletons
- **`AMFITRACK_Devices`** ([src/Amfitrack_Devices.h](src/Amfitrack_Devices.h)) — device registry; stores `AMFITRACK_Sensor` and `AMFITRACK_Source` in `unordered_map<uint8_t, ...>` keyed by device ID (0–254; 255 is broadcast). Overloaded `set()` methods update individual fields.
- **`AMFITRACK_Config`** ([src/Amfitrack_config.h](src/Amfitrack_config.h)) — state-machine for config discovery (`CONFIG_DISCOVERY_IDLE` → `DONE`), started by `AMFITRACK::getConfiguration()`.
- **`AMFITRACK_ResetInfo`** ([src/Amfitrack_resetinfo.h](src/Amfitrack_resetinfo.h)) — state-machine for the paged ResetInfo exchange (`RESET_INFO_IDLE` → `DONE` / `FAILED`), started by `AMFITRACK::requestResetInfo(id, count)`. Walks the newest `count` records of the device's fault log, re-entering the per-record states once per record. Single-flight: one device at a time, and it gives up after 3 attempts per request rather than retrying until the device disconnects the way config discovery does. A record that will not read is skipped, not treated as the end of the log; only record 0 failing ends the walk.
- **`amfitrack_task`** ([src/Amfitrack_task.h](src/Amfitrack_task.h)) — periodic keep-alive pings and version/name polling for connected devices
- **`AmfiProt_API`** ([lib/amfiprotapi/lib_AmfiProt_API.hpp](lib/amfiprotapi/lib_AmfiProt_API.hpp)) — protocol layer; extends both `lib_AmfiProt` (framing/CRC) and `lib_AmfiProt_AmfiTrack` (AMFITRACK-specific message handlers). Uses two FIFOs (`outgoingBulk_FiFo`, `incomingBulk_FiFo`) for frame queuing.
- **`HIDMonitor`** ([drv/drv_USB/HID_Monitor.h](drv/drv_USB/HID_Monitor.h)) — scans USB for AMFITRACK devices (VID `0x0C17`, sensor PID `0x0D12`, source PID `0x0D01`), handles read/write. Communicates upward via `HIDMonitorCallbacks` function objects.

`AMFITRACK_Config` and `AMFITRACK_ResetInfo` each expose a `run()` that is stepped from `_run_all_amfitrack()` in [Amfitrack.cpp](Amfitrack.cpp), so both advance under either driving mode above. Do not confuse these with the public `AMFITRACK::run()` — callers never call the per-singleton ones directly.

### Data Flow
```
HIDMonitor (USB read) → AmfiProt_API::deserialize_frame()
  → libAmfiProt_handle_* / lib_AmfiProt_Amfitrack_handle_* callbacks
    → AMFITRACK_Devices::set(...) updates sensor/source fields
      → Caller reads via AMFITRACK::get_sensor() / get_source()
```

Outbound: `AmfiProt_API::queue_frame()` → `HIDMonitor::drainTxQueue()` → USB write.

### Device Data Structs
All types are defined in [AmfitrackDeviceTypes.h](AmfitrackDeviceTypes.h):
- **`AMFITRACK_Sensor`** — holds `Pose_t`, `IMU_t`, `Sensor_Status_t`, `External_input_t`, raw/normalized B-field data, `DeviceConfig_t`, `ResetInfoLog_t`
- **`AMFITRACK_Source`** — holds `IMU_t`, `Source_Status_t`, `Current_t`, `Voltage_t`, `Frequency_t`, `Calibration_t`, `DeviceConfig_t`, `ResetInfoLog_t`

`ResetInfoLog_t` holds the records a walk read, in index order (0 = newest), plus the device's own `count`. Like `DeviceConfig_t` it owns a `std::vector`, so `reset()` assigns a fresh value rather than `memset`ing it. It is written only when the walk reaches a terminal state — on `DONE`, and on `FAILED` if any record had already been assembled, so a failed walk can still yield records. Read it via `AMFITRACK::getResetInfoLog()`, or one record by index via `AMFITRACK::getResetInfo()`, which reports the `valid` flag rather than whether the field exists.

### Support Libraries (`lib/`)
- **`lib_log`** — `Log::init(level)` sets verbosity; use `LOG_E/W/I/D(fmt, ...)` macros throughout
- **`lib_time`** — `lib_time::get_time_ms()` wraps platform time; prefer this over `<chrono>` directly
- **`lib_generic_parameter`** — typed config value (`lib_Generic_Parameter_Value_t`). Use `GenericParameter::f32(v)`, `GenericParameter::u8(v)`, etc. as factory methods rather than filling the struct manually
- **`lib_fifo`** — header-only templated FIFO
- **`lib_crc`** — CRC utilities used by the protocol layer

### Threading
When `USE_THREAD_BASED` is defined, each singleton that holds state has a `mutable std::mutex _mutex`. The background thread is started via `AMFITRACK::start_task()` which calls `background_amfitrack_task()`. Without threading, `run()` must be called in a loop.
