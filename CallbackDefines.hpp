#pragma once

namespace Babel
{
	struct LogInInfo {
		const char* User;
		int UserLen;
		const char* Password;
		int PasswordLen;
		int StoreCredentials;
	};

	struct NewAccountInfo {
		const char* User;
		int UserLen;
		const char* Password;
		int PasswordLen;
		const char* Name;
		int NameLen;
		const char* Surname;
		int SurnameLen;
	};

	struct SingleStringParam {
		const char* Str;
		int Len;
	};

	struct DoubleStringParam {
		const char* FirstStr;
		int FirstLen;
		const char* SecondStr;
		int SecondLen;		
	};

	struct TripleStringParam {
		const char* FirstStr;
		int FirstLen;
		const char* SecondStr;
		int SecondLen;
		const char* ThirdStr;
		int ThirdLen;
	};

	typedef void(__stdcall* TLogInCallback)(LogInInfo* loginInfo);
	typedef void(__stdcall* TCreateAccountCallback)(NewAccountInfo* accountInfo);
	typedef void(__stdcall* TCloseClient)(void);
	typedef void(__stdcall* TSetHost)(SingleStringParam* host);
	typedef void(__stdcall* TValidateAccount)(DoubleStringParam* host);
	typedef void(__stdcall* TResendValidationCode)(SingleStringParam* host);
	typedef void(__stdcall* TRequestPasswordReset)(SingleStringParam* host);
	typedef void(__stdcall* TNewPasswordRequest)(TripleStringParam* host);
	typedef void(__stdcall* TSelectCharacter)(int);
	typedef void(__stdcall* TLoginCharacterIndex)(int);
	

	struct CallbacksList
	{
		TLogInCallback Login;
		TCreateAccountCallback CreateAccount;
		TCloseClient CloseClient;
		TSetHost SetHost;
		TValidateAccount ValidateAccount;
		TResendValidationCode ResendValidationCode;
		TRequestPasswordReset RequestPasswordReset;
		TNewPasswordRequest NewPasswordRequest;
		TSelectCharacter SelectCharacter;
		TLoginCharacterIndex LoginWithCharacter;
	};

}