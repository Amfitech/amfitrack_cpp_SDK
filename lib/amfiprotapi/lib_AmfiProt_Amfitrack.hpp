///-----------------------------------------------------------------------------
//                              AMFITECH APS
//                          ALL RIGHTS RESERVED
//-----------------------------------------------------------------------------

#ifndef LIB_AMFIPROT_AMFITRACK_H_
#define LIB_AMFIPROT_AMFITRACK_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
/// \cond
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "lib_AmfiProt.hpp"

#ifndef __PACKED_STRUCT
#include <cmsis_compiler.h>
#endif

/// \endcond

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Type declarations
//-----------------------------------------------------------------------------

typedef enum
{
//	lib_AmfiProt_Amfitrack_PayloadType_SourceCoilData = 0x10,                   // TODO: Remove
//	lib_AmfiProt_Amfitrack_PayloadType_RequestCalibrationSignal = 0x11,         // TODO: Remove when converted to RPC        // Sent from sensor to source to start offset coil switching

//	lib_AmfiProt_Amfitrack_PayloadType_Calibrate = 0x12,                        // TODO: Remove when converted to RPC       // Sent to sensor, to perform a startup, PLL or IMU calibration
//	lib_AmfiProt_Amfitrack_PayloadType_Calibrate_If_Uncalibrated = 0x13,        // TODO: Remove when converted to RPC       // Sent to sensor, to perform a calibration, if the sensor is currently uncalibrated

#ifdef AMFIPROT_AMFITRACK_LEGACY_PACKETS
	lib_AmfiProt_Amfitrack_PayloadType_EMF = 0x14, // TODO: Remove
#endif
//	lib_AmfiProt_Amfitrack_PayloadType_EMF_Timestamp = 0x15,                    // TODO: Remove
#ifdef AMFIPROT_AMFITRACK_LEGACY_PACKETS
	lib_AmfiProt_Amfitrack_PayloadType_EMF_IMU = 0x16, // TODO: Remove
#endif

	//	lib_AmfiProt_Amfitrack_PayloadType_EMF_IMU_Timestamp = 0x17,                // TODO: Remove
	//	lib_AmfiProt_Amfitrack_PayloadType_IMU = 0x18,                              // TODO: Remove
	//	lib_AmfiProt_Amfitrack_PayloadType_IMU_Timestamp = 0x19,                    // TODO: Remove
	lib_AmfiProt_Amfitrack_PayloadType_Sensor_Measurement = 0x1A, // lib_AmfiProt_Amfitrack_Sensor_Measurement_t
	lib_AmfiProt_Amfitrack_PayloadType_Sensor_Status = 0x1B,	  // lib_AmfiProt_Amfitrack_Sensor_Status_t

	//	lib_AmfiProt_Amfitrack_PayloadType_SetSendIMU = 0x20,                       // TODO: Remove
	lib_AmfiProt_Amfitrack_PayloadType_SourceCoilCalData = 0x21,  // TODO: Remove
																  //	lib_AmfiProt_Amfitrack_PayloadType_SourceCoilCalIMUData = 0x22,             // TODO: Remove
	lib_AmfiProt_Amfitrack_PayloadType_Source_Calibration = 0x23, // lib_AmfiProt_Amfitrack_Source_Calibration_t
	lib_AmfiProt_Amfitrack_PayloadType_Source_Measurement = 0x24, // lib_AmfiProt_Amfitrack_Source_Measurement_t

	//	lib_AmfiProt_Amfitrack_PayloadType_MetaData = 0x80,						    // TODO: Remove     // For Occulus Raspberry Pi integration
	//	lib_AmfiProt_Amfitrack_PayloadType_EMF_Meta = 0x81,                         // TODO: Remove     // For Occulus Raspberry Pi integration
	//	lib_AmfiProt_Amfitrack_PayloadType_EMF_IMU_Meta = 0x82,						// TODO: Remove     // For Occulus Raspberry Pi integration

	//	lib_AmfiProt_Amfitrack_PayloadType_RawBfieldX = 0xA0,                       // TODO: Remove
	//	lib_AmfiProt_Amfitrack_PayloadType_RawBfieldY = 0xA1,                       // TODO: Remove
	//	lib_AmfiProt_Amfitrack_PayloadType_RawBfieldZ = 0xA2,                       // TODO: Remove
	//	lib_AmfiProt_Amfitrack_PayloadType_RawBfieldRef = 0xA3,                     // TODO: Remove
	lib_AmfiProt_Amfitrack_PayloadType_Raw_Bfield = 0xA4,				  // lib_AmfiProt_Amfitrack_Sensor_BField_Raw_t
	lib_AmfiProt_Amfitrack_PayloadType_Normalized_Bfield = 0xA5,		  // lib_AmfiProt_Amfitrack_Sensor_BField_Raw_t
	lib_AmfiProt_Amfitrack_PayloadType_Bfield_With_Phase = 0xA6,		  // lib_AmfiProt_Amfitrack_Sensor_BField_With_Phase_t
	lib_AmfiProt_Amfitrack_PayloadType_Normalized_Bfield_With_IMU = 0xA7, // lib_AmfiProt_Amfitrack_Sensor_BField_With_IMU_t
																		  //	lib_AmfiProt_Amfitrack_PayloadType_CalibratedBfieldX = 0xB3,                // TODO: Remove
																		  //	lib_AmfiProt_Amfitrack_PayloadType_CalibratedBfieldY = 0xB4,                // TODO: Remove
																		  //	lib_AmfiProt_Amfitrack_PayloadType_CalibratedBfieldZ = 0xB5,                // TODO: Remove
																		  //	lib_AmfiProt_Amfitrack_PayloadType_SourceCrossTalk = 0xC0,                  // TODO: Remove

	//	lib_AmfiProt_Amfitrack_PayloadType_RawADCSamples = 0xE0,                    // TODO: Remove
	lib_AmfiProt_Amfitrack_PayloadType_Raw_Data = 0xE1, // lib_AmfiProt_Amfitrack_Raw_Data_t

	//	lib_AmfiProt_Amfitrack_PayloadType_StopCalibrationSignal = 0xE2,            // TODO: Remove     // Sent from sensor to source to stop offset coil switching
	lib_AmfiProt_Amfitrack_PayloadType_SetPhaseModulation = 0xE3, // TODO: Remove when converted to RPC
	lib_AmfiProt_Amfitrack_PayloadType_Sign_Data = 0xE4,		  // lib_AmfiProt_Amfitrack_Sign_Data_t
	lib_AmfiProt_Amfitrack_PayloadType_Pll_Data = 0xE5,			  // lib_AmfiProt_Amfitrack_Pll_Data_t
} lib_AmfiProt_Amfitrack_PayloadType_t;

// -----------------------------------------------
//  Structures for use in application (Not packed)
// ------------------------------------------------
typedef struct
{
	float position_x_in_m;
	float position_y_in_m;
	float position_z_in_m;
	float orientation_x;
	float orientation_y;
	float orientation_z;
	float orientation_w;
} lib_AmfiProt_Amfitrack_Pose_t;

typedef struct
{
	float acceleration_x_in_mg;
	float acceleration_y_in_mg;
	float acceleration_z_in_mg;
	float rotation_x_in_rad_per_sec;
	float rotation_y_in_rad_per_sec;
	float rotation_z_in_rad_per_sec;
} lib_AmfiProt_Amfitrack_IMU_t;

typedef struct
{
	float magneto_X_in_uT;
	float magneto_Y_in_uT;
	float magneto_Z_in_uT;
} lib_AmfiProt_Amfitrack_magneto_t;

typedef struct
{
	float current_coil_x_in_mA;
	float current_coil_y_in_mA;
	float current_coil_z_in_mA;
} lib_AmfiProt_Amfitrack_Source_Coil_Current_t;

typedef uint32_t lib_AmfiProt_Amfitrack_FrameID_t;

// -------------------------------------------------
//  Structures shared between multiple packet types
// -------------------------------------------------

#ifdef _MSC_VER
#pragma pack(push, 1)
#define LIB_AMFIPROT_AMFITRACK_PACKED_PUSHED
#endif

typedef __PACKED_STRUCT
{
	uint8_t value[3];
}
lib_AmfiProt_Amfitrack_FrameID_24b_t;

typedef __PACKED_STRUCT
{
	/** 4000mg full scale (Multiply with 4/(2^15-1) to get result in g) */
	int16_t acceleration_X;

	/** 4000mg full scale (Multiply with 4/(2^15-1) to get result in g) */
	int16_t acceleration_Y;

	/** 4000mg full scale (Multiply with 4/(2^15-1) to get result in g) */
	int16_t acceleration_Z;

	/** LSB = 70mdeg/s (Multiply with 0.070*PI/180 to get result in rad/sec) */
	int16_t rotation_X;

	/** LSB = 70mdeg/s (Multiply with 0.070*PI/180 to get result in rad/sec) */
	int16_t rotation_Y;

	/** LSB = 70mdeg/s (Multiply with 0.070*PI/180 to get result in rad/sec) */
	int16_t rotation_Z;
}
lib_AmfiProt_Amfitrack_IMU_16b_t;

typedef __PACKED_STRUCT
{
	/** 400 uT full scale (Multiply with 400/(2^15-1) to get result in uT) */
	int16_t magneto_X;

	/** 400 uT full scale (Multiply with 400/(2^15-1) to get result in uT) */
	int16_t magneto_Y;

	/** 400 uT full scale (Multiply with 400/(2^15-1) to get result in uT) */
	int16_t magneto_Z;
}
lib_AmfiProt_Amfitrack_magneto_16b_t;

typedef __PACKED_STRUCT
{
	/** LSB = 0.01mm (Multiply with 100000 to get results in m) */
	uint8_t position_x[3];

	/** LSB = 0.01mm (Multiply with 100000 to get results in m) */
	uint8_t position_y[3];

	/** LSB = 0.01mm (Multiply with 100000 to get results in m) */
	uint8_t position_z[3];

	/** LSB = 1/1000000 (Multiply with 1000000 to get unit quaternion) */
	uint8_t orientation_x[3];

	/** LSB = 1/1000000 (Multiply with 1000000 to get unit quaternion) */
	uint8_t orientation_y[3];

	/** LSB = 1/1000000 (Multiply with 1000000 to get unit quaternion) */
	uint8_t orientation_z[3];

	/** LSB = 1/1000000 (Multiply with 1000000 to get unit quaternion) */
	uint8_t orientation_w[3];
}
lib_AmfiProt_Amfitrack_Pose_24b_t;

// ---------------------------------
//  Structures for protocol packets
// ---------------------------------

/**
 * Sensor package, with position, IMU and status. (PayloadType == 0x1A)
 */
typedef __PACKED_STRUCT
{
	/** Position and orientation */
	lib_AmfiProt_Amfitrack_Pose_24b_t pose;

	/** Sensor status ( \ref sensorStatus ) */
	uint8_t sensor_status;

	/** Received source ID */
	uint8_t source_id;

	/** Calculation ID */
	uint16_t calculation_id;

	/** IMU data */
	lib_AmfiProt_Amfitrack_IMU_16b_t imu_data;

	/** Magnetometer data */
	lib_AmfiProt_Amfitrack_magneto_16b_t magneto_data;

	/** Magnetometer temperature */
	uint8_t temperature; // Divide by 2 and subtract 30 to get temperature in degree C (-30 to 97.5 C range)

	/** Current sensor state (See \ref SensorState_t ) */
	uint8_t sensor_state;

	/** Metal distortion (0 == Most distortion | 255 == Least distortion)*/
	uint8_t metal_distortion;

	/** GPIO State */
	uint16_t gpio_state;

	/** rssi */
	int8_t rssi;

	/** Frame ID */
	lib_AmfiProt_Amfitrack_FrameID_24b_t frame_id;
}
lib_AmfiProt_Amfitrack_Sensor_Measurement_t;
// static_assert(sizeof(lib_AmfiProt_Amfitrack_Sensor_Measurement_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Sensor_Measurement_t larger than max payload size");

/* New low rate sensor package, with more detailed status */
/* Work in progress */
typedef __PACKED_STRUCT
{
	/** Battery SOC (%) */
	uint8_t bat_SOC;

	/** rssi */
	int8_t rssi;

	/** Frame ID */
	lib_AmfiProt_Amfitrack_FrameID_24b_t frame_id;
	
	/** Application state (see \ref AppState_t) */
	uint8_t sensor_appState;
}
lib_AmfiProt_Amfitrack_Sensor_Status_t;

/**
 * Source calibration package (PayloadType == 0x23)
 */
typedef __PACKED_STRUCT
{
	/** X coil frequency */
	float frequency_x_in_Hz;

	/** Y coil frequency */
	float frequency_y_in_Hz;

	/** Z coil frequency */
	float frequency_z_in_Hz;

	/** X coil calibration value */
	float calibration_source_coil_x;

	/** Y coil calibration value */
	float calibration_source_coil_y;

	/** Z coil calibration value */
	float calibration_source_coil_z;

	/** Phase modulation offset X coil, used by sensor */
	float phase_modulation_offset_x;

	/** Phase modulation offset Y coil, used by sensor */
	float phase_modulation_offset_y;

	/** Phase modulation offset Z coil, used by sensor */
	float phase_modulation_offset_z;
}
lib_AmfiProt_Amfitrack_Source_Calibration_t;
static_assert(sizeof(lib_AmfiProt_Amfitrack_Source_Calibration_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Source_Calibration_t larger than max payload size");

/**
 * Source package, with current, IMU and status (PayloadType == 0x24)
 */
typedef __PACKED_STRUCT
{
	/** X coil current in mA */
	float current_coil_x_in_mA;

	/** Y coil current in mA */
	float current_coil_y_in_mA;

	/** Z coil current in mA */
	float current_coil_z_in_mA;

	/** IMU data */
	lib_AmfiProt_Amfitrack_IMU_16b_t imu_data;

	/** Magnetometer measurements */
	lib_AmfiProt_Amfitrack_magneto_16b_t magneto_data;

	/** Magnetometer temperature */
	uint8_t temperature; // Divide by 2 and subtract 30 to get temperature in degree C (-30 to 97.5 C range)

	/** Source Status (TODO) */
	uint8_t source_status; // Used to indicate battery status, sync to optitrack, hardware failures (broken wire)

	/** Source State: \n Bit 1 == Phase modulation \n Bit 2 == Frequency tune \n Bit 3 == Current tune \n Bit 4 == Crosstalk calibration \n
	 *  Bit 5 == Offset coil enabled */
	uint8_t source_state; // on/off state for: Phase modulation, offset coil, data over B-field, (maybe also cross talk calibration, frequency tuning, current tuning)

	/** rssi */
	int8_t rssi;

	/** Frame ID */
	lib_AmfiProt_Amfitrack_FrameID_24b_t frame_id;

	/** X coil voltage */
	float voltage_coil_x_in_V;

	/** Y coil voltage */
	float voltage_coil_y_in_V;

	/** Z coil voltage */
	float voltage_coil_z_in_V;

	/** Boost voltage */
	float voltage_boost_in_V;
}
lib_AmfiProt_Amfitrack_Source_Measurement_t;
// static_assert(sizeof(lib_AmfiProt_Amfitrack_Source_Measurement_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Source_Measurement_t larger than max payload size");

typedef __PACKED_STRUCT
{
	float bfield[9];
	float source_current[3];

	uint8_t sensor_status;
	uint8_t source_id;
	lib_AmfiProt_Amfitrack_FrameID_24b_t frame_id;
}
lib_AmfiProt_Amfitrack_Sensor_BField_t;
// static_assert(sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Sensor_BField_t larger than max payload size");

typedef __PACKED_STRUCT
{
	float bfield[9];
	int8_t phase[9];

	uint8_t sensor_status;
	uint8_t source_id;
	lib_AmfiProt_Amfitrack_FrameID_24b_t frame_id;
}
lib_AmfiProt_Amfitrack_Sensor_BField_With_Phase_t;
static_assert(sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_With_Phase_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Sensor_BField_With_Phase_t larger than max payload size");

typedef __PACKED_STRUCT
{
	float bfield[9];
	uint8_t sensor_status;
	uint8_t source_id;
	lib_AmfiProt_Amfitrack_IMU_16b_t imu_data;
	uint8_t metal_distortion;
	lib_AmfiProt_Amfitrack_FrameID_24b_t frame_id;
}
lib_AmfiProt_Amfitrack_Sensor_BField_With_IMU_t;
// static_assert(sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_With_IMU_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Sensor_BField_With_IMU_t larger than max payload size");

typedef __PACKED_STRUCT
{
	uint8_t coil_index;
	uint8_t reference_channel;
	float pll_frequency;
	float phase_division;
	float remainder;
	float offset;
	bool sign_stable;
}
lib_AmfiProt_Amfitrack_Sign_Data_t;
static_assert(sizeof(lib_AmfiProt_Amfitrack_Sign_Data_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Sign_Data_t larger than max payload size");

typedef __PACKED_STRUCT
{
	uint8_t coil_index;
	float freq;
	float amplitude[3];
	float phase[3];
}
lib_AmfiProt_Amfitrack_Pll_Data_t;
static_assert(sizeof(lib_AmfiProt_Amfitrack_Pll_Data_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Pll_Data_t larger than max payload size");

typedef __PACKED_STRUCT
{
	float data[13];
}
lib_AmfiProt_Amfitrack_Raw_Data_t;
static_assert(sizeof(lib_AmfiProt_Amfitrack_Raw_Data_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_Raw_Data_t larger than max payload size");

typedef __PACKED_STRUCT
{
	float currentCoilX;
	float currentCoilY;
	float currentCoilZ;
	float frequencyX;
	float frequencyY;
	float frequencyZ;
	float CalSrcX;
	float CalSrcY;
	float CalSrcZ;
}
lib_AmfiProt_Amfitrack_SourceCoilCalData_t;
static_assert(sizeof(lib_AmfiProt_Amfitrack_SourceCoilCalData_t) <= AmfiProtMaxPayloadLength, "lib_AmfiProt_Amfitrack_SourceCoilCalData_t larger than max payload size");

#ifdef LIB_AMFIPROT_AMFITRACK_PACKED_PUSHED
#pragma pack(pop)
#undef LIB_AMFIPROT_AMFITRACK_PACKED_PUSHED
#endif

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
class lib_AmfiProt_AmfiTrack
{
  private:
  public:
	lib_AmfiProt_AmfiTrack();
	~lib_AmfiProt_AmfiTrack();

	void lib_AmfiProt_Amfitrack_processFrame(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle);
	void lib_AmfiProt_Amfitrack_SendSourceCoilCalData(lib_AmfiProt_Amfitrack_SourceCoilCalData_t *data);

	void lib_AmfiProt_Amfitrack_SendPoseExt(lib_AmfiProt_Amfitrack_Pose_t *pose, uint8_t source_id, uint8_t sensorStatus, lib_AmfiProt_Amfitrack_IMU_t *imu, lib_AmfiProt_Amfitrack_magneto_t *magneto, float temperature, uint32_t frame_id, uint16_t calcID, uint16_t gpioState, uint8_t metalDistortion, uint8_t sensorState);
	void lib_AmfiProt_Amfitrack_SendRawFloats(lib_AmfiProt_Amfitrack_Raw_Data_t *data);
	void lib_AmfiProt_Amfitrack_SendRawBfield(lib_AmfiProt_Amfitrack_Sensor_BField_t *raw_bfield, uint32_t frame_id);
	void lib_AmfiProt_Amfitrack_SendNormBfield(lib_AmfiProt_Amfitrack_Sensor_BField_t *data, uint32_t frame_id);
	void lib_AmfiProt_Amfitrack_SendNormBfield_with_imu(lib_AmfiProt_Amfitrack_Sensor_BField_t *bfield, lib_AmfiProt_Amfitrack_IMU_t *imu, uint8_t metalDistortion, uint32_t frame_id);
	void lib_AmfiProt_Amfitrack_SendBfieldPhase(lib_AmfiProt_Amfitrack_Sensor_BField_With_Phase_t *data, uint32_t frame_id);
	void lib_AmfiProt_Amfitrack_SendSignData(lib_AmfiProt_Amfitrack_Sign_Data_t *data);
	void lib_AmfiProt_Amfitrack_SendPllData(lib_AmfiProt_Amfitrack_Pll_Data_t *data);

	void lib_AmfiProt_Amfitrack_SendSourceMeasurement(lib_AmfiProt_Amfitrack_Source_Coil_Current_t *current, lib_AmfiProt_Amfitrack_IMU_t *imu, lib_AmfiProt_Amfitrack_magneto_t *magneto, float temperature, uint32_t frame_id, uint8_t source_state, uint8_t source_status);
	void lib_AmfiProt_Amfitrack_SendSourceCalibration(lib_AmfiProt_Amfitrack_Source_Calibration_t *data);

	void lib_AmfiProt_Amfitrack_SetSourceID(uint8_t sourceID);
	void lib_AmfiProt_Amfitrack_SetPhaseModulation(uint8_t sourceID, bool enable);

	void lib_AmfiProt_Amfitrack_encode_pose_i24(lib_AmfiProt_Amfitrack_Pose_t const *pose, lib_AmfiProt_Amfitrack_Pose_24b_t *pose_24b);
	void lib_AmfiProt_Amfitrack_encode_frame_id(lib_AmfiProt_Amfitrack_FrameID_24b_t *frame_id_24b, uint32_t frame_id);
	void lib_AmfiProt_Amfitrack_encodeIMU_i16(lib_AmfiProt_Amfitrack_IMU_t const *imu, lib_AmfiProt_Amfitrack_IMU_16b_t *imu_16b);
	void lib_AmfiProt_Amfitrack_encode_magneto_i16(lib_AmfiProt_Amfitrack_magneto_t const *magneto, lib_AmfiProt_Amfitrack_magneto_16b_t *magneto_16b);
	float lib_AmfiProt_Amfitrack_decode_temperature_u8(uint8_t temperature_u8);
	uint8_t lib_AmfiProt_Amfitrack_encode_temperature_u8(float temperature);

	void lib_AmfiProt_Amfitrack_decodeIMU_i16(lib_AmfiProt_Amfitrack_IMU_16b_t const *imu_16b, lib_AmfiProt_Amfitrack_IMU_t *imu);
	void lib_AmfiProt_Amfitrack_decode_magneto_i16(lib_AmfiProt_Amfitrack_magneto_16b_t const *mag_16b, lib_AmfiProt_Amfitrack_magneto_t *mag);
	uint32_t lib_AmfiProt_Amfitrack_decode_frame_id(lib_AmfiProt_Amfitrack_FrameID_24b_t const *frame_id_24b);
	void lib_AmfiProt_Amfitrack_decode_pose_i24(lib_AmfiProt_Amfitrack_Pose_24b_t const *pose_24b, lib_AmfiProt_Amfitrack_Pose_t *pose);

	virtual void lib_AmfiProt_Amfitrack_handle_SourceCalibration(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_SourceMeasurement(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_SensorMeasurement(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
	{
		(void)handle;
		(void)frame;
		(void)routing_handle;
	};
	virtual void lib_AmfiProt_Amfitrack_handle_SensorStatus(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_RawBfield(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_NormalizedBfield(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_BfieldPhase(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_NormalizedBfieldImu(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_SignData(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_PllData(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_RawFloats(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_SetPhaseModulation(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_SourceCoilCalData(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
	virtual void lib_AmfiProt_Amfitrack_handle_AlternativeProcessing(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle) = 0;
};

//-----------------------------------------------------------------------------
// Variables and constants
//-----------------------------------------------------------------------------

#endif // LIB_AMFIPROT_AMFITRACK_H_H
