#pragma once

namespace Babel
{
	struct LogInInfo {
		const char* user;
		int userLen;
		const char* password;
		int passwordLen;
	};

	typedef void(__stdcall* TLogInCallback)(LogInInfo* loginInfo);
	typedef void(__stdcall* TCloseClient)(void);
	

	struct CallbacksList
	{
		TLogInCallback login;
		TCloseClient closeClient;
	};

}