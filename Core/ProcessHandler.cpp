#include "ProcessHandler.hpp"

ProcessHandler::~ProcessHandler()
{
    CloseProcess();
}

bool ProcessHandler::StartProcess(const char * process)
{
	ZeroMemory(&mStartupInfo, sizeof(mStartupInfo));
	mStartupInfo.cb = sizeof(mStartupInfo);
	ZeroMemory(&mStartupInfo, sizeof(mStartupInfo));
    return CreateProcess(NULL,   // No module name (use command line)
        const_cast<char*>(process),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &mStartupInfo,            // Pointer to STARTUPINFO structure
        &mProcessInfo);           // Pointer to PROCESS_INFORMATION structure
}

void ProcessHandler::CloseProcess()
{
	CloseHandle(mProcessInfo.hProcess);
	CloseHandle(mProcessInfo.hThread);
}
