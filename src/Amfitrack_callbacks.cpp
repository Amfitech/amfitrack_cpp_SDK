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
#include "lib_AmfiProt_API.hpp"
#include "Amfitrack.h"
#include "AmfitrackDeviceTypes.h"
#include "Amfitrack_config.h"
#include "Amfitrack_resetinfo.h"
#include "Amfitrack_Devices.h"
#include "lib_log.h"
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
//-----------------------------------------------------------------------------
// Section: Functions
//-----------------------------------------------------------------------------
void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_SourceCalibration(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	lib_AmfiProt_Amfitrack_Source_Calibration_t sourceCalibration;
	memcpy(&sourceCalibration, &frame->payload[0], sizeof(lib_AmfiProt_Amfitrack_Source_Calibration_t));

	uint8_t _deviceID = frame->header.source;

	Frequency_t freq;
	freq.Frequency_X = sourceCalibration.frequency_x_in_Hz;
	freq.Frequency_Y = sourceCalibration.frequency_y_in_Hz;
	freq.Frequency_Z = sourceCalibration.frequency_z_in_Hz;
	AMFITRACK_Devices::getInstance().set(_deviceID, freq);

	Calibration_t cal;
	cal.Calibration_X = sourceCalibration.calibration_source_coil_x;
	cal.Calibration_Y = sourceCalibration.calibration_source_coil_y;
	cal.Calibration_Z = sourceCalibration.calibration_source_coil_z;
	AMFITRACK_Devices::getInstance().set(_deviceID, cal);
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_SourceMeasurement(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;

	lib_AmfiProt_Amfitrack_Source_Measurement_t sourceMeasurement;
	memcpy(&sourceMeasurement, &frame->payload[0], sizeof(lib_AmfiProt_Amfitrack_Source_Measurement_t));

	Current_t cur;
	cur.Current_X = sourceMeasurement.current_coil_x_in_mA / 1000;
	cur.Current_Y = sourceMeasurement.current_coil_y_in_mA / 1000;
	cur.Current_Z = sourceMeasurement.current_coil_z_in_mA / 1000;
	AMFITRACK_Devices::getInstance().set(_deviceID, cur);

	Voltage_t vol;
	vol.Voltage_X = sourceMeasurement.voltage_coil_x_in_V;
	vol.Voltage_Y = sourceMeasurement.voltage_coil_y_in_V;
	vol.Voltage_Z = sourceMeasurement.voltage_coil_z_in_V;
	vol.Voltage_Boost = sourceMeasurement.voltage_boost_in_V;
	AMFITRACK_Devices::getInstance().set(_deviceID, vol);

	lib_AmfiProt_Amfitrack_IMU_t tempIMU;
	lib_AmfiProt_Amfitrack_decodeIMU_i16(&sourceMeasurement.imu_data, &tempIMU);
	IMU_t imu;
	imu.Acceleration_X = tempIMU.acceleration_x_in_mg / 1000;
	imu.Acceleration_Y = tempIMU.acceleration_y_in_mg / 1000;
	imu.Acceleration_Z = tempIMU.acceleration_z_in_mg / 1000;
	imu.Rotation_X = tempIMU.rotation_x_in_rad_per_sec;
	imu.Rotation_Y = tempIMU.rotation_y_in_rad_per_sec;
	imu.Rotation_Z = tempIMU.rotation_z_in_rad_per_sec;
	AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Source, imu);

	Source_Status_t status;
	status.RSSI = -sourceMeasurement.rssi;
	status.Temperature = (float)(((float)sourceMeasurement.temperature / 2) - 30);
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_SensorMeasurement(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;

	lib_AmfiProt_Amfitrack_Sensor_Measurement_t SensorMeasurement;
	memcpy(&SensorMeasurement, &frame->payload[0], sizeof(lib_AmfiProt_Amfitrack_Sensor_Measurement_t));

	lib_AmfiProt_Amfitrack_Pose_t tempPose;
	lib_AmfiProt_Amfitrack_decode_pose_i24(&SensorMeasurement.pose, &tempPose);
	Pose_t pose = { tempPose.position_x_in_m, tempPose.position_y_in_m, tempPose.position_z_in_m, tempPose.orientation_x, tempPose.orientation_y, tempPose.orientation_z, tempPose.orientation_w };
	AMFITRACK_Devices::getInstance().set(_deviceID, pose);

	lib_AmfiProt_Amfitrack_IMU_t tempIMU;
	lib_AmfiProt_Amfitrack_decodeIMU_i16(&SensorMeasurement.imu_data, &tempIMU);
	IMU_t imu;
	imu.Acceleration_X = tempIMU.acceleration_x_in_mg / 1000;
	imu.Acceleration_Y = tempIMU.acceleration_y_in_mg / 1000;
	imu.Acceleration_Z = tempIMU.acceleration_z_in_mg / 1000;
	imu.Rotation_X = tempIMU.rotation_x_in_rad_per_sec;
	imu.Rotation_Y = tempIMU.rotation_y_in_rad_per_sec;
	imu.Rotation_Z = tempIMU.rotation_z_in_rad_per_sec;
	AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Sensor, imu);

	AMFITRACK_Devices::getInstance().set(_deviceID, SensorMeasurement);

	AMFITRACK_Sensor sensor;
	AMFITRACK_Devices::getInstance().get_sensor_by_id(_deviceID, &sensor);

	Sensor_Status_t status = sensor.status;
	status.RSSI = -SensorMeasurement.rssi;
	status.Temperature = (float)(((float)SensorMeasurement.temperature / 2) - 30);
	status.Field_Quality = (SensorMeasurement.metal_distortion * 100) / 255;

	status.Battery_Charging = (SensorMeasurement.sensor_status & (1U << 2)) != 0;
	status.Source_Connected = (SensorMeasurement.sensor_status & (1U << 3)) != 0;
	status.B_Field_Status = (BFieldStatus_t)((SensorMeasurement.sensor_status >> 4) & 0x07);
	status.Pose_state = (PoseState_t)SensorMeasurement.sensor_state;
	AMFITRACK_Devices::getInstance().set(_deviceID, status);

	External_input_t extInput;
	extInput.ADC_input = ((float)((SensorMeasurement.gpio_state >> 4) & 0x0FFF) / 1000);
	extInput.GPIO_1 = (SensorMeasurement.gpio_state & (1 << 0)) ? 1 : 0;
	extInput.GPIO_2 = (SensorMeasurement.gpio_state & (1 << 1)) ? 1 : 0;
	extInput.GPIO_3 = (SensorMeasurement.gpio_state & (1 << 2)) ? 1 : 0;
	extInput.GPIO_4 = (SensorMeasurement.gpio_state & (1 << 3)) ? 1 : 0;
	AMFITRACK_Devices::getInstance().set(_deviceID, extInput);
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_SensorStatus(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	uint8_t _frameLen = frame->header.length;
	lib_AmfiProt_Amfitrack_Sensor_Status_t SensorStatus = {};
	memcpy(&SensorStatus, &frame->payload[0], _frameLen);

	AMFITRACK_Sensor sensor;
	AMFITRACK_Devices::getInstance().get_sensor_by_id(_deviceID, &sensor);

	Sensor_Status_t status = sensor.status;
	status.Battery_SOC = SensorStatus.bat_SOC;
	if (_frameLen == sizeof(lib_AmfiProt_Amfitrack_Sensor_Status_t))
	{
		status.App_state = (AppState_t)SensorStatus.sensor_appState;
	}	

	AMFITRACK_Devices::getInstance().set(_deviceID, status);
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_RawBfield(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_Amfitrack_Sensor_BField_t bfield;
	memcpy(&bfield, &frame->payload[0], sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_t));
	Raw_B_Field_t rawBfield;
	memcpy(&rawBfield.bfield, bfield.bfield, sizeof(bfield.bfield));
	AMFITRACK_Devices::getInstance().set(_deviceID, rawBfield);
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_NormalizedBfield(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_Amfitrack_Sensor_BField_t bfield;
	memcpy(&bfield, &frame->payload[0], sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_t));
	Normalized_B_Field_t normBfield;
	memcpy(&normBfield.bfield, bfield.bfield, sizeof(bfield.bfield));
	AMFITRACK_Devices::getInstance().set(_deviceID, normBfield);
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_BfieldPhase(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_Amfitrack_Sensor_BField_With_Phase_t bfield;
	memcpy(&bfield, &frame->payload[0], sizeof(lib_AmfiProt_Amfitrack_Sensor_BField_With_Phase_t));
	Raw_with_Phase_B_Field_t rawWithPhaseBfield;
	memcpy(&rawWithPhaseBfield.bfield, bfield.bfield, sizeof(bfield.bfield));
	AMFITRACK_Devices::getInstance().set(_deviceID, rawWithPhaseBfield);
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_NormalizedBfieldImu(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_SignData(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_PllData(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_RawFloats(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_SetPhaseModulation(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_SourceCoilCalData(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::lib_AmfiProt_Amfitrack_handle_AlternativeProcessing(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestProcedureSpec(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ReplyProcedureSpec(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestProcedureCall(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ReplyProcedureCall(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestDeviceID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RespondDeviceID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_DeviceID idFrame;
	memcpy(&idFrame, frame->payload, sizeof(idFrame));

	AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Both, true);
	AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Both, idFrame.UUID[0], idFrame.UUID[1], idFrame.UUID[2]);
}

void AmfiProt_API::libAmfiProt_handle_SetTxID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestFirmwareVersion(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_FirmwareVersion(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_FirmwareStart(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_FirmwareData(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_FirmwareEnd(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestDeviceName(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ReplyDeviceName(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	size_t str_length = strnlen((char *)(&(frame->payload[1])), MAX_PAYLOAD_SIZE - 1);

	AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Both, (char *)(&(frame->payload[1])), (uint8_t)str_length);
}

void AmfiProt_API::libAmfiProt_handle_RequestConfigurationValue(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ReplyConfigurationValue(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_SetConfigurationValue(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestConfigurationName(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ReplyConfigurationName(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_LoadDefault(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_SaveAsDefault(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestConfigurationNameAndUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ConfigurationNameAndUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_ConfigNameUID_protocol_t amfiConfigNameUID;
	memcpy(&amfiConfigNameUID, frame->payload, frame->header.length);
	AMFITRACK_Config::getInstance().set(_deviceID, amfiConfigNameUID);
}

void AmfiProt_API::libAmfiProt_handle_RequestConfigurationValueUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ConfigurationValueUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_ConfigValueUID_t amfiConfigValueUID;
	memcpy(&amfiConfigValueUID, frame->payload, sizeof(amfiConfigValueUID));
	AMFITRACK_Config::getInstance().set(_deviceID, amfiConfigValueUID);
}

void AmfiProt_API::libAmfiProt_handle_SetConfigurationValueUID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestConfigurationCategory(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ConfigurationCategory(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_ConfigCategory_t amfiConfigCategory;
	memcpy(&amfiConfigCategory, frame->payload, frame->header.length);
	AMFITRACK_Config::getInstance().set(_deviceID, amfiConfigCategory);
}

void AmfiProt_API::libAmfiProt_handle_RequestConfigurationValueCount(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ConfigurationValueCount(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_ConfigValueCount_t amfiConfigValueCount;
	memcpy(&amfiConfigValueCount, frame->payload, sizeof(amfiConfigValueCount));
	AMFITRACK_Config::getInstance().set(_deviceID, amfiConfigValueCount);
}

void AmfiProt_API::libAmfiProt_handle_RequestCategoryCount(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_CategoryCount(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_ConfigCategoryCount_t amfiConfigCategoryCount;
	memcpy(&amfiConfigCategoryCount, frame->payload, sizeof(amfiConfigCategoryCount));
	AMFITRACK_Config::getInstance().set(_deviceID, amfiConfigCategoryCount);
}

void AmfiProt_API::libAmfiProt_handle_Reboot(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_DebugOutput(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ResetParameter(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_RequestFirmwareVersionPerID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ReplyFirmwareVersionPerID(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;
	lib_AmfiProt_FirmwareVersionPerID_t FirmwareVersion;
	memcpy(&FirmwareVersion, frame->payload, sizeof(lib_AmfiProt_FirmwareVersionPerID_t));

	switch (FirmwareVersion.processorID)
	{
		case 0:
			FW_t fw;
			fw.Major = FirmwareVersion.major;
			fw.Minor = FirmwareVersion.minor;
			fw.Patch = FirmwareVersion.patch;
			fw.Build = FirmwareVersion.build;
			AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Both, fw);
			break;
		case 1:
			RF_t rf;
			rf.Major = FirmwareVersion.major;
			rf.Minor = FirmwareVersion.minor;
			rf.Patch = FirmwareVersion.patch;
			rf.Build = FirmwareVersion.build;
			AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Both, rf);
			break;
		case 255:
			HW_t hw;
			hw.Generation = FirmwareVersion.major;
			hw.Version = FirmwareVersion.minor;
			hw.SubVersion = FirmwareVersion.patch;
			hw.Frequency = FirmwareVersion.build;
			AMFITRACK_Devices::getInstance().set(_deviceID, AMFITRACK_Devices::deviceType_t::Both, hw);
			break;
	}
}

void AmfiProt_API::libAmfiProt_handle_RequestResetInfo(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)frame;
	(void)routing_handle;
	/* NOTE: Overwrite in application-specific library */
}

void AmfiProt_API::libAmfiProt_handle_ReplyResetInfo(void *handle, lib_AmfiProt_Frame_t *frame, void *routing_handle)
{
	(void)handle;
	(void)routing_handle;
	uint8_t _deviceID = frame->header.source;

	// Both reply shapes share payload ID 0x1F; the field byte at offset 1 says which.
	if (frame->header.length < 2U)
	{
		LOG_W("ReplyResetInfo: device %u sent %u byte(s), too short to demux", _deviceID, frame->header.length);
		return;
	}

	switch (frame->payload[1])
	{
		case lib_AmfiProt_ResetInfoField_Summary:
		{
			if (frame->header.length != sizeof(lib_AmfiProt_ResetInfoSummary_t))
			{
				LOG_W("ReplyResetInfo: device %u summary is %u byte(s), need %u",
					  _deviceID, frame->header.length, (unsigned)sizeof(lib_AmfiProt_ResetInfoSummary_t));
				return;
			}

			lib_AmfiProt_ResetInfoSummary_t resetInfoSummary;
			memcpy(&resetInfoSummary, frame->payload, sizeof(resetInfoSummary));
			AMFITRACK_ResetInfo::getInstance().set(_deviceID, resetInfoSummary);
		}
		break;
		case lib_AmfiProt_ResetInfoField_File:
		case lib_AmfiProt_ResetInfoField_Func:
		case lib_AmfiProt_ResetInfoField_Expr:
		{
			// The last chunk of a field is short, so copy the frame length rather
			// than the struct size, and carry the length through: text is not
			// NUL terminated on the wire.
			lib_AmfiProt_ResetInfoChunk_t resetInfoChunk = {};
			memcpy(&resetInfoChunk, frame->payload, frame->header.length);
			AMFITRACK_ResetInfo::getInstance().set(_deviceID, resetInfoChunk, frame->header.length);
		}
		break;
		default:
			LOG_W("ReplyResetInfo: device %u sent unknown field %u", _deviceID, frame->payload[1]);
			break;
	}
}
