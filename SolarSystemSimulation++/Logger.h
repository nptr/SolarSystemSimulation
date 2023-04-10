#pragma once

#include <deque>
#include <string>

enum LogLevel
{
	DEBUG,
	INFO,
	WARN,
	ERROR,
	NONE
};


struct LogItem
{
	LogLevel Level;
	std::string Text;
};


typedef std::deque<LogItem> Logs;


class Logger
{
public:
	static Logger& GetInstance();

	void Log(LogLevel level, const char* format, ...);

	Logs& GetLogs() { return m_log; }

private:
	Logger() { m_level = LogLevel::INFO; }
	Logger(Logger const&);
	void operator=(Logger const&) = delete;

	LogLevel m_level;
	Logs m_log;
};