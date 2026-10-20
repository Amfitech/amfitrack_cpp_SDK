#include "lib_log.h"

//-----------------------------------------------------------------------------
// Section: Variables
//-----------------------------------------------------------------------------
LogLevel Log::m_level = LOG_LEVEL_INFO;

//-----------------------------------------------------------------------------
// Section: Public
//-----------------------------------------------------------------------------
void Log::init(LogLevel level)
{
	m_level = level;
}

void Log::e(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	print(LOG_LEVEL_ERROR, "ERROR", fmt, args);

	va_end(args);
}

void Log::w(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	print(LOG_LEVEL_WARNING, "WARN", fmt, args);

	va_end(args);
}

void Log::i(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	print(LOG_LEVEL_INFO, "INFO", fmt, args);

	va_end(args);
}

void Log::d(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	print(LOG_LEVEL_DEBUG, "DEBUG", fmt, args);

	va_end(args);
}

//-----------------------------------------------------------------------------
// Section: Private
//-----------------------------------------------------------------------------
void Log::print(LogLevel msgLevel,
				const char *levelText,
				const char *fmt,
				va_list args)
{
	if (msgLevel > m_level)
	{
		return;
	}

	printf("[%s] ", levelText);
	vprintf(fmt, args);
	printf("\n");
}