//-----------------------------------------------------------------------------
//
//                              AMFITECH APS
//
//                          ALL RIGHTS RESERVED
//
//-----------------------------------------------------------------------------

#pragma once
#ifdef __cplusplus
//-----------------------------------------------------------------------------
// Section: Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include "AmfitrackDeviceTypes.h"
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
// Section: Class
//-----------------------------------------------------------------------------

class amfitrack_task
{
	enum class missingInfo_t
	{
		missingInfo_FW,
		missingInfo_RF,
		missingInfo_HW,
		missingInfo_Name,
	};

  public:
	static void init();
	static void run();

  private:
	static void keepAlivePing();
	static void checkDisconnected();

	static void getVersion(uint8_t deviceID, uint8_t _version);
	static void getName(uint8_t deviceID);
	static void getMissingInfo();
	static void requestMissingInfo(uint8_t deviceID, const FW_t &fw, const RF_t &rf,
								   const HW_t &hw, const char *name);
	static missingInfo_t missingInfo;
};

#endif
/** @} */ // end of module
