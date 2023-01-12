#include "Logger.hpp"
#include <time.h>
#include <stdarg.h>
#include "StringUtils.hpp"
#include <inttypes.h>
#include <stdexcept>

#pragma warning(disable : 4996) // no es que me guste desactivar warnings pero esto es algo de vs y no del standard del lenguaje
#define MAX_FMT_TRIES    5   // #of times we try
#define FMT_BLOCK_SIZE    2048 // # of bytes to increment per try
#define BUFSIZE_1ST  256
#define BUFSIZE_2ND 512
#define STD_BUF_SIZE    1024

#if defined(_WIN32) && !defined(va_copy)
#define va_copy(dst, src) ((dst) = (src))
#endif


Logger * Logger::instance = NULL;
const std::string k_separator("================================================================================");
static const char* prefixFormat = "%02.2d:%02.2d:%02.2d T:%" PRIu64 " %7s: ";

void FormatV(std::string & source,const char* szFormat, va_list argList)
{
    // try and grab a sufficient buffersize
    int nChars = FMT_BLOCK_SIZE;
    va_list argCopy;

	char *p = new char[nChars];
    if (!p) return;

    while (1)
    {
        va_copy(argCopy, argList);

        int nActual = ssvsprintf(p, nChars, szFormat, argCopy);
        /* If that worked, return the string. */
        if (nActual > -1 && nActual < nChars)
        { /* make sure it's NULL terminated */
			p[nActual] = '\0';
			source.assign(p, nActual);
			delete [] p;
			va_end(argCopy);
			return;
        }
        /* Else try again with more space. */
        if (nActual > -1)        /* glibc 2.1 */
			nChars = nActual + 1;  /* precisely what is needed */
        else                     /* glibc 2.0 */
			nChars *= 2;           /* twice the old size */

		char *np = new char[nChars];
		delete [] p;
        if (np == NULL)
        {
			va_end(argCopy);
			return;   // failed :(
        }
        p = np;
        va_end(argCopy);
    }
}

Logger * Logger::Get()
{
	if( instance == NULL)
	{
		instance = new Logger();
	}
	return instance;
}

void Logger::Destroy()
{
	if(instance == NULL) return;
	delete instance;
	instance = NULL;
}

Logger::Logger()
{
	processName.reserve(255);
	tabSize = 0;
}


Logger::~Logger()
{
}

void Logger::init(const std::string & pFileName,const std::string & pProcess)
{
	processName = "[" + pProcess + "] ";
	tabStartPos = processName.length();
    logFileName = pFileName;
}

void Logger::log(const std::string & pLine)
{
    auto logFile = fopen(logFileName.c_str(), "a");
	if( logFile != NULL)
	{
		std::string line( processName );
		time_t lTime;
		time(&lTime);
		struct tm * localTime = localtime( &lTime );
		char timeBuff[24]; 
		strftime(timeBuff, 24, "[%d/%m/%y %H:%M:%S]: ", localTime);
		line += timeBuff;
		line += tab;
        line += pLine + "\n";
        std::lock_guard<std::mutex> lock(mWriteFileMutex);
		fputs( line.c_str() , logFile);
		fflush(logFile);
        fclose(logFile);
	}
}

void Logger::log(const char *format, ... )
{
    auto logFile = fopen(logFileName.c_str(), "a");
    if( logFile != NULL)
	{
		std::string line( processName );
		time_t lTime;
		time(&lTime);
		struct tm * localTime = localtime( &lTime );
		char timeBuff[24]; 
		strftime(timeBuff, 24, "[%d/%m/%y %H:%M:%S]: ", localTime);
		line += timeBuff;
		line += tab;
		std::string stdData;
		stdData.reserve(16384);
        va_list va;
        va_start(va, format);
        FormatV(stdData,format,va);
        va_end(va);
		line += stdData + "\n";
        std::lock_guard<std::mutex> lock(mWriteFileMutex);
		fputs( line.c_str() , logFile);
		fflush(logFile);
        fclose(logFile);
	}
}

void Logger::addSeparator( bool pWithDate )
{
	std::string line( processName );
    auto logFile = fopen(logFileName.c_str(), "a");
	if( logFile != NULL)
	{
	
		if( pWithDate )
		{
			time_t lTime;
			time(&lTime);
			struct tm * localTime = localtime( &lTime );
			char timeBuff[24]; 
			strftime(timeBuff, 24, "[%d/%m/%y %H:%M:%S]: ", localTime);
			line += timeBuff;
			line += tab;
		}
		line += k_separator + "\n";
        std::lock_guard<std::mutex> lock(mWriteFileMutex);
		fputs( line.c_str() , logFile);
		fflush(logFile);
        fclose(logFile);
	}
    
}

void Logger::skipLine( int pCount)
{
    auto logFile = fopen(logFileName.c_str(), "a");
	if( logFile != NULL)
	{
        std::lock_guard<std::mutex> lock(mWriteFileMutex);
		for( int i = 0; i < pCount; i++)
		{
			fputs( "\n", logFile);
		}
		fflush(logFile);
        fclose(logFile);
	}
    
}

void Logger::addTabSize( size_t pCharCount)
{
	tabSize += pCharCount;
	tab = std::string(tabSize, ' ');
}

void Logger::removeTabSize( size_t pCharCount)
{
	if( pCharCount > tabSize )
	{
		tabSize = 0;
	}
	else
	{
		tabSize -= pCharCount;
	}
	tab = std::string(tabSize, ' ');
}

Logger * Logger::Create()
{
	return new Logger;
}