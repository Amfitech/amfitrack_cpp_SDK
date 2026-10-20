//-----------------------------------------------------------------------------
//                              AMFITECH APS
//                          ALL RIGHTS RESERVED
//
// $URL: $
// $Rev: $
// $Date: $
// $Author: $
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
/// \cond
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h> // For memcpy
/// \endcond

#include "lib_AmfiProt_Amfitrack.hpp"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
static int32_t const INT24_MAX = 8388607;
static int32_t const INT24_MIN = -8388608;

//-----------------------------------------------------------------------------
// Type declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variables and constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
lib_AmfiProt_AmfiTrack::lib_AmfiProt_AmfiTrack()
{
}

lib_AmfiProt_AmfiTrack::~lib_AmfiProt_AmfiTrack()
{
}

void lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_processFrame(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	switch (frame->header.payloadType)
	{
		case lib_AmfiProt_Amfitrack_PayloadType_Source_Calibration:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Source_Calibration_t))
			{
				lib_AmfiProt_Amfitrack_handle_SourceCalibration(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Source_Measurement:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Source_Measurement_t))
			{
				lib_AmfiProt_Amfitrack_handle_SourceMeasurement(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Sensor_Measurement:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Sensor_Measurement_t))
			{
				lib_AmfiProt_Amfitrack_handle_SensorMeasurement(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Sensor_Status:
		{
			if (frame->header.length <= sizeof(lib_AmfiProt_Amfitrack_Sensor_Status_t))
			{
				lib_AmfiProt_Amfitrack_handle_SensorStatus(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Raw_Bfield:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_t))
			{
				lib_AmfiProt_Amfitrack_handle_RawBfield(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Normalized_Bfield:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_t))
			{
				lib_AmfiProt_Amfitrack_handle_NormalizedBfield(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Bfield_With_Phase:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_With_Phase_t))
			{
				lib_AmfiProt_Amfitrack_handle_BfieldPhase(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Normalized_Bfield_With_IMU:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_With_IMU_t))
			{
				lib_AmfiProt_Amfitrack_handle_NormalizedBfieldImu(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Sign_Data:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Sign_Data_t))
			{
				lib_AmfiProt_Amfitrack_handle_SignData(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Pll_Data:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Pll_Data_t))
			{
				lib_AmfiProt_Amfitrack_handle_PllData(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_Raw_Data:
		{
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_Raw_Data_t))
			{
				lib_AmfiProt_Amfitrack_handle_RawFloats(handle, frame, routing_handle);
			}
		}
		break;
		case lib_AmfiProt_Amfitrack_PayloadType_SetPhaseModulation:
			if (frame->header.length == 1)
			{
				lib_AmfiProt_Amfitrack_handle_SetPhaseModulation(handle, frame, routing_handle);
			}
			// else
			//{
			//     libAmfiProt_ReplyInvalid(handle, frame, routing_handle);
			// }
			break;
		case lib_AmfiProt_Amfitrack_PayloadType_SourceCoilCalData:
			if (frame->header.length == sizeof(lib_AmfiProt_Amfitrack_SourceCoilCalData_t))
			{
				lib_AmfiProt_Amfitrack_SourceCoilCalData_t sourceCoilCalData;
				memcpy(&sourceCoilCalData, frame->payload, sizeof(lib_AmfiProt_Amfitrack_SourceCoilCalData_t));
				lib_AmfiProt_Amfitrack_handle_SourceCoilCalData(handle, frame, routing_handle);
			}
			break;

		default:
			lib_AmfiProt_Amfitrack_handle_AlternativeProcessing(handle, frame, routing_handle);
			break;
	}
}

uint32_t lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_decode_frame_id(lib_AmfiProt_Amfitrack_FrameID_24b_t const *frame_id_24b)
{
	assert(frame_id_24b);

	uint32_t retVal = (uint32_t)(frame_id_24b->value[0]) | (uint32_t)(frame_id_24b->value[1]) << 8 | (uint32_t)(frame_id_24b->value[2]) << 16;
	return retVal;
}

void encode_frame_id(lib_AmfiProt_Amfitrack_FrameID_24b_t *frame_id_24b, uint32_t frame_id)
{
	assert(frame_id_24b);

	frame_id_24b->value[0] = (uint8_t)(frame_id & 0xFF);
	frame_id_24b->value[1] = (uint8_t)((frame_id & 0xFF << 8) >> 8);
	frame_id_24b->value[2] = (uint8_t)((frame_id & 0xFF << 16) >> 16);
}

static float convert_i24_to_f32(uint8_t const *element_i24, double scaleFactor)
{
	int32_t result_i32;
	double result = 0.0f;

	result_i32 = (uint32_t)(element_i24[0]);
	result_i32 |= (uint32_t)(element_i24[1] << 8);
	result_i32 |= (uint32_t)(element_i24[2] << 16);
	if (result_i32 & 0x800000)
	{
		result_i32 |= ~0xFFFFFF;
	}
	result = (float)result_i32 * scaleFactor;

	return (float)result;
}

static int32_t convert_f32_to_i24(float element, uint8_t *element_i24, float scaleFactor)
{
	int32_t result = 0;
	double scaledElement = (double)element * scaleFactor;

	if (scaledElement >= INT24_MAX)
	{
		result = INT24_MAX;
	}
	else if (scaledElement <= INT24_MIN)
	{
		result = INT24_MIN;
	}
	else
	{
		result = (int32_t)scaledElement;
	}

	if (element_i24)
	{
		element_i24[2] = (result >> 16) & 0xff;
		element_i24[1] = (result >> 8) & 0xff;
		element_i24[0] = (result) & 0xff;
	}

	return result;
}

void lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_decodeIMU_i16(lib_AmfiProt_Amfitrack_IMU_16b_t const *imu_16b, lib_AmfiProt_Amfitrack_IMU_t *imu)
{
	const float acceleration_scale = 0.1220740f; // 4000/(2^15-1) : 4000mg full scale range
	imu->acceleration_x_in_mg = (float)(imu_16b->acceleration_X) * acceleration_scale;
	imu->acceleration_y_in_mg = (float)(imu_16b->acceleration_Y) * acceleration_scale;
	imu->acceleration_z_in_mg = (float)(imu_16b->acceleration_Z) * acceleration_scale;

	const float rotation_scale = 0.0012217f; // 0.070 * PI/180 : (70mdps/LSB), measured in radians per seconds
	imu->rotation_x_in_rad_per_sec = (float)(imu_16b->rotation_X) * rotation_scale;
	imu->rotation_y_in_rad_per_sec = (float)(imu_16b->rotation_Y) * rotation_scale;
	imu->rotation_z_in_rad_per_sec = (float)(imu_16b->rotation_Z) * rotation_scale;
}

void lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_encodeIMU_i16(lib_AmfiProt_Amfitrack_IMU_t const *imu, lib_AmfiProt_Amfitrack_IMU_16b_t *imu_16b)
{
	const float acceleration_scale = 8.19175f; // (2^15-1)/4000 : 4000mg full scale range
	imu_16b->acceleration_X = (int16_t)(imu->acceleration_x_in_mg * acceleration_scale);
	imu_16b->acceleration_Y = (int16_t)(imu->acceleration_y_in_mg * acceleration_scale);
	imu_16b->acceleration_Z = (int16_t)(imu->acceleration_z_in_mg * acceleration_scale);

#ifdef SEND_MAGNETO_INSTEAD_OF_GYRO
	const float rotation_scale = 81.9175f; // (2^15-1)/400 : 400 uT full scale range
	imu_16b->rotation_X = (int16_t)(imu->rotation_x_in_rad_per_sec * rotation_scale);
	imu_16b->rotation_Y = (int16_t)(imu->rotation_y_in_rad_per_sec * rotation_scale);
	imu_16b->rotation_Z = (int16_t)(imu->rotation_z_in_rad_per_sec * rotation_scale);
#else
	const float rotation_scale = 818.51114f; // 1/(0.070 * PI/180) : (70mdps/LSB), measured in radians per seconds
	imu_16b->rotation_X = (int16_t)(imu->rotation_x_in_rad_per_sec * rotation_scale);
	imu_16b->rotation_Y = (int16_t)(imu->rotation_y_in_rad_per_sec * rotation_scale);
	imu_16b->rotation_Z = (int16_t)(imu->rotation_z_in_rad_per_sec * rotation_scale);
#endif
}

void lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_decode_magneto_i16(lib_AmfiProt_Amfitrack_magneto_16b_t const *mag_16b, lib_AmfiProt_Amfitrack_magneto_t *mag)
{
	const float mag_scale = 0.01220740f; // 400/(2^15-1) : 400 uT full scale range
	mag->magneto_X_in_uT = (float)(mag_16b->magneto_X) * mag_scale;
	mag->magneto_Y_in_uT = (float)(mag_16b->magneto_Y) * mag_scale;
	mag->magneto_Z_in_uT = (float)(mag_16b->magneto_Z) * mag_scale;
}

void lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_encode_magneto_i16(lib_AmfiProt_Amfitrack_magneto_t const *magneto, lib_AmfiProt_Amfitrack_magneto_16b_t *magneto_16b)
{
	const float magneto_scale = 81.9175f; // (2^15-1)/400 : 400 uT full scale range
	magneto_16b->magneto_X = (int16_t)(magneto->magneto_X_in_uT * magneto_scale);
	magneto_16b->magneto_Y = (int16_t)(magneto->magneto_Y_in_uT * magneto_scale);
	magneto_16b->magneto_Z = (int16_t)(magneto->magneto_Z_in_uT * magneto_scale);
}

float lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_decode_temperature_u8(uint8_t temperature_u8)
{
	float temperature_f32 = 0.0f;

	temperature_f32 = 0.5f * ((float)temperature_u8) - 30.0f;

	return temperature_f32;
}

uint8_t lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_encode_temperature_u8(float temperature)
{
	uint8_t temperature_u8 = 0;

	temperature = (temperature + 30.0f) * 2.0f;
	if (temperature > 255.0f)
	{
		temperature_u8 = 255;
	}
	else if (temperature < 0.0f)
	{
		temperature_u8 = 0;
	}
	else
	{
		temperature_u8 = (uint8_t)temperature;
	}

	return temperature_u8;
}

void lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_decode_pose_i24(lib_AmfiProt_Amfitrack_Pose_24b_t const *pose_24b, lib_AmfiProt_Amfitrack_Pose_t *pose)
{
	const double position_scale = 0.00001;
	pose->position_x_in_m = convert_i24_to_f32(pose_24b->position_x, position_scale);
	pose->position_y_in_m = convert_i24_to_f32(pose_24b->position_y, position_scale);
	pose->position_z_in_m = convert_i24_to_f32(pose_24b->position_z, position_scale);

	const double orientation_scale = 0.000001;
	pose->orientation_x = convert_i24_to_f32(pose_24b->orientation_x, orientation_scale);
	pose->orientation_y = convert_i24_to_f32(pose_24b->orientation_y, orientation_scale);
	pose->orientation_z = convert_i24_to_f32(pose_24b->orientation_z, orientation_scale);
	pose->orientation_w = convert_i24_to_f32(pose_24b->orientation_w, orientation_scale);
}

void lib_AmfiProt_AmfiTrack::lib_AmfiProt_Amfitrack_encode_pose_i24(lib_AmfiProt_Amfitrack_Pose_t const *pose, lib_AmfiProt_Amfitrack_Pose_24b_t *pose_24b)
{
	const double position_scale = 100000.0;
	convert_f32_to_i24(pose->position_x_in_m, pose_24b->position_x, position_scale);
	convert_f32_to_i24(pose->position_y_in_m, pose_24b->position_y, position_scale);
	convert_f32_to_i24(pose->position_z_in_m, pose_24b->position_z, position_scale);

	const double orientation_scale = 1000000.0;
	convert_f32_to_i24(pose->orientation_x, pose_24b->orientation_x, orientation_scale);
	convert_f32_to_i24(pose->orientation_y, pose_24b->orientation_y, orientation_scale);
	convert_f32_to_i24(pose->orientation_z, pose_24b->orientation_z, orientation_scale);
	convert_f32_to_i24(pose->orientation_w, pose_24b->orientation_w, orientation_scale);
}
