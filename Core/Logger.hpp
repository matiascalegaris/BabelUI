#pragma once
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string>
#include <mutex>
#ifdef __GNUC__
#define ATTRIB_LOG_FORMAT __attribute__((format(printf,2,3)))
#else
#define ATTRIB_LOG_FORMAT
#endif

namespace Babel
{
	class Logger
	{
	public:
		static Logger* Get();
		static void		Destroy();

		static Logger* Create();

		void init(const std::string& pFileName, const std::string& pProcess);
		void log(const std::string& pLine);
		void log(const char* format, ...) ATTRIB_LOG_FORMAT;

		void addSeparator(bool pWithDate = true);
		void skipLine(int pCount = 1);

		void addTabSize(size_t pCharCount);
		void removeTabSize(size_t pCharCount);

		const std::string& getLogFileName() const { return logFileName; };
	private:
		Logger();
		~Logger();
		static Logger* instance;

		size_t tabSize;
		size_t tabStartPos;

		std::string tab;
		std::string processName;
		std::string logFileName;
		std::mutex mWriteFileMutex;
	};

#define LOGGER Logger::Get()

	class ContextLog
	{
	public:
		ContextLog(const std::string& pContext) : context(pContext)
		{
			LOGGER->log(context + " start");
			LOGGER->addTabSize(4);
		}
		~ContextLog()
		{
			LOGGER->removeTabSize(4);
			LOGGER->log(context + " exit");
		}
	private:
		std::string context;
	};

	class ContextSeparator
	{
	public:
		ContextSeparator()
		{
			LOGGER->addSeparator(true);
		}
		~ContextSeparator()
		{
			LOGGER->addSeparator(true);
		}
	};
	class LoggerGuard
	{
	public:
		~LoggerGuard()
		{
			Logger::Destroy();
		}
	};
}
#endif