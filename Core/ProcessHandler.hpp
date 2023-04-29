#pragma once

#include <Windows.h>

namespace Babel {

	class ProcessHandler
	{
	public:
		~ProcessHandler();
		bool StartProcess(const char* process);
		void CloseProcess();

	private:
		STARTUPINFO mStartupInfo;
		PROCESS_INFORMATION mProcessInfo;
	};
}