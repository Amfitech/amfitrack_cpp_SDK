//-----------------------------------------------------------------------------
//
//                              AMFITECH APS
//
//                          ALL RIGHTS RESERVED
//
//-----------------------------------------------------------------------------
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#include "lib_Generic_Parameter.h"

#ifndef MAX_NAME_LENGTH
#define MAX_NAME_LENGTH 64
#endif

//-----------------------------------------------------------------------------
// General enum types
//-----------------------------------------------------------------------------
typedef enum
{
	BFIELD_STATUS_TRACKING = 0,
	BFIELD_STATUS_UNCERTAIN_TRACKING = 1,
	BFIELD_STATUS_NO_TRACKING = 2,
	BFIELD_STATUS_TOO_LOW = 3,
	BFIELD_STATUS_TOO_HIGH = 4,
	BFIELD_STATUS_SOURCE_COIL_DEFECT = 5,
	BFIELD_STATUS_SENSOR_COIL_DEFECT = 6
} BFieldStatus_t;

typedef enum
{
	POSE_STATE_NO_TRACKING = 0,
	POSE_STATE_TRACKING = 1,
	POSE_STATE_UNCERTAIN_TRACKING = 2,
	POSE_STATE_POTENTIAL_STARTUP_POSITION = 3,
	POSE_STATE_AWAITING_LOCK = 4,
	POSE_STATE_FINDING_HEMISPHERE = 5,
	POSE_STATE_PLL_CALIBRATION_NPA = 6,
	POSE_STATE_PLL_CALIBRATION_PA = 7,
	POSE_STATE_TRACKING_VERIFYING_HEMISPHERE = 8
} PoseState_t;

typedef enum
{
	APP_STATE_POWERUP = 0,
	APP_STATE_INITIALIZE = 1,
	APP_STATE_RUNNING = 2,
	APP_STATE_CHARGING = 3,
	APP_STATE_POWERDOWN = 4,
	APP_STATE_FIRMWAREUPDATE = 5,
	APP_STATE_LAST,
} AppState_t;


//-----------------------------------------------------------------------------
// General struct types
//-----------------------------------------------------------------------------
typedef struct
{
	uint32_t Major;
	uint32_t Minor;
	uint32_t Patch;
	uint32_t Build;
} FW_t;

typedef struct
{
	uint32_t Major;
	uint32_t Minor;
	uint32_t Patch;
	uint32_t Build;
} RF_t;

typedef struct
{
	uint32_t Generation;
	uint32_t Version;
	uint32_t SubVersion;
	uint32_t Frequency;
} HW_t;

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------
typedef struct
{
	uint32_t uid;
	char name[45];
	uint8_t categoryIndex;
	lib_Generic_Parameter_Value config;
} ConfigEntry_t;

typedef struct
{
	uint8_t index;
	char name[52];
	uint16_t configCount;
	std::vector<ConfigEntry_t> configs;
} CategoryEntry_t;

typedef struct
{
	uint8_t categoryCount;
	std::vector<CategoryEntry_t> categories;
} DeviceConfig_t;

//-----------------------------------------------------------------------------
// Sensor struct types
//-----------------------------------------------------------------------------
typedef struct
{
	/** Battery state of charge in percent (0-100) */
	uint32_t Battery_SOC;
	/** True if battery is charging */
	bool Battery_Charging;

	/** Received signal strength indicator (dBm) */
	int8_t RSSI;
	/** Source temperature in degrees Celsius */
	float Temperature;
	/** Field quality in percent (0-100) */
	uint8_t Field_Quality;

	bool Source_Connected;
	BFieldStatus_t 	B_Field_Status;
	PoseState_t 	Pose_state;
	AppState_t 		App_state;
} Sensor_Status_t;

typedef struct
{
	/** Received signal strength indicator (dBm) */
	int8_t RSSI;
	/** Source temperature in degrees Celsius */
	float Temperature;
} Source_Status_t;

typedef struct
{
	/** External analog input voltage */
	float ADC_input;

	bool GPIO_1;
	bool GPIO_2;
	bool GPIO_3;
	bool GPIO_4;
} External_input_t;

typedef struct
{
	float Position_X;	 // In meter
	float Position_Y;	 // In meter
	float Position_Z;	 // In meter
	float Orientation_X; // In quaternion
	float Orientation_Y; // In quaternion
	float Orientation_Z; // In quaternion
	float Orientation_W; // In quaternion
} Pose_t;

typedef struct
{
	float Acceleration_X; // In g
	float Acceleration_Y; // In g
	float Acceleration_Z; // In g
	float Rotation_X;	  // In radian per second
	float Rotation_Y;	  // In radian per second
	float Rotation_Z;	  // In radian per second
} IMU_t;

typedef struct
{
	float Magneto_X; // In micro tesla
	float Magneto_Y; // In micro tesla
	float Magneto_Z; // In micro tesla
} Magneto_t;

typedef struct
{
	float bfield[9]; // In raw ADC count
} Raw_B_Field_t;

typedef struct
{
	float bfield[9]; // In calibrated value
} Normalized_B_Field_t;

typedef struct
{
	float bfield[9]; // In raw ADC count
	float phase[9];
} Raw_with_Phase_B_Field_t;

//-----------------------------------------------------------------------------
// Source struct types
//-----------------------------------------------------------------------------
typedef struct
{
	float Current_X; // In Amp
	float Current_Y; // In Amp
	float Current_Z; // In Amp
} Current_t;

typedef struct
{
	float Voltage_X;	 // In Voltage
	float Voltage_Y;	 // In Voltage
	float Voltage_Z;	 // In Voltage
	float Voltage_Boost; // In Voltage
} Voltage_t;

typedef struct
{
	float Frequency_X; // In Hz
	float Frequency_Y; // In Hz
	float Frequency_Z; // In Hz
} Frequency_t;

typedef struct
{
	float Calibration_X;
	float Calibration_Y;
	float Calibration_Z;
} Calibration_t;

/** @} */ // end of module
