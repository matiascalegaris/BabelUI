#include "ProcessHandler.hpp"
#include "Logger.hpp"

namespace Babel {
    ProcessHandler::~ProcessHandler()
    {
        CloseProcess();
    }

    bool ProcessHandler::StartProcess(const char* process)
    {
        try {
            ZeroMemory(&mStartupInfo, sizeof(mStartupInfo));
            mStartupInfo.cb = sizeof(mStartupInfo);
            ZeroMemory(&mStartupInfo, sizeof(mProcessInfo));
            if (CreateProcess(NULL,   // No module name (use command line)
                const_cast<char*>(process),        // Command line
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                CREATE_NO_WINDOW,              // No creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory 
                &mStartupInfo,            // Pointer to STARTUPINFO structure
                &mProcessInfo) == 0)           // Pointer to PROCESS_INFORMATION structure
            {
                LOGGER->log("Failed to open child process");
                return false;
            }
            DWORD ProccessStatus;
            if (!GetExitCodeProcess(mProcessInfo.hProcess, &ProccessStatus))
            {
                LOGGER->log("Failed to get child process exit code");
                return false;
            }
            return ProccessStatus == STILL_ACTIVE;
        }
        catch (...)
        {
            LOGGER->log("Failed to open child process");
            return false;
        }
    }

    void ProcessHandler::CloseProcess()
    {
        CloseHandle(mProcessInfo.hProcess);
        CloseHandle(mProcessInfo.hThread);
    }
}