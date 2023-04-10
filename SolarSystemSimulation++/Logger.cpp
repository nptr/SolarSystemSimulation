#include "Logger.h"

#include <stdio.h>
#include <stdarg.h>

#define EVENT_LOG_MAX 50

Logger& Logger::GetInstance()
{
	static Logger instance;
	return instance;
}


void Logger::Log(LogLevel level, const char* format, ...)
{
	char buffer[256] = { 0 };

	if (level < m_level)
		return;

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, 255, format, args);

	LogItem item;
	item.Level = level;
	item.Text = buffer;
	m_log.push_back(item);

	if (m_log.size() > EVENT_LOG_MAX)
	{
		m_log.pop_front();
	}

    fprintf(stderr, "%s", item.Text.c_str());

	va_end(args);
}