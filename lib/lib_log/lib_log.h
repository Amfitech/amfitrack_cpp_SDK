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
#include <stdio.h>
#include <stdarg.h>

//-----------------------------------------------------------------------------
// Section: Define
//-----------------------------------------------------------------------------
#define LOG_E(...) Log::e(__VA_ARGS__)
#define LOG_W(...) Log::w(__VA_ARGS__)
#define LOG_I(...) Log::i(__VA_ARGS__)
#define LOG_D(...) Log::d(__VA_ARGS__)

//-----------------------------------------------------------------------------
// Section: Enum
//-----------------------------------------------------------------------------
enum LogLevel
{
	LOG_LEVEL_NONE = 0,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG
};
//-----------------------------------------------------------------------------
// Section: Class
//-----------------------------------------------------------------------------

class Log
{
  public:
	static void init(LogLevel level);
	static void e(const char *fmt, ...);
	static void w(const char *fmt, ...);
	static void i(const char *fmt, ...);
	static void d(const char *fmt, ...);

  private:
	static void print(LogLevel msgLevel,
					  const char *levelText,
					  const char *fmt,
					  va_list args);

	static LogLevel m_level;
};

#endif