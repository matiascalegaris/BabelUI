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

	struct NewCharacterInfo {
		const char* NameStr;
		int NameLen;
		int Gender;
		int Race;
		int Class;
		int Head;
		int City;
	};

	typedef void(__stdcall* TLogInCallback)(LogInInfo*);
	typedef void(__stdcall* TCreateAccountCallback)(NewAccountInfo*);
	typedef void(__stdcall* TVoidParam)(void);
	typedef void(__stdcall* TSetHost)(SingleStringParam*);
	typedef void(__stdcall* TValidateAccount)(DoubleStringParam* );
	typedef void(__stdcall* TResendValidationCode)(SingleStringParam*);
	typedef void(__stdcall* TRequestPasswordReset)(SingleStringParam*);
	typedef void(__stdcall* TNewPasswordRequest)(TripleStringParam*);
	typedef void(__stdcall* TSelectCharacter)(int);
	typedef void(__stdcall* TLoginCharacterIndex)(int);
	typedef void(__stdcall* TCreateCharacter)(NewCharacterInfo*);
	typedef void(__stdcall* TIntStringF)(int, SingleStringParam*);

	struct CallbacksList
	{
		TLogInCallback Login;
		TCreateAccountCallback CreateAccount;
		TVoidParam CloseClient;
		TSetHost SetHost;
		TValidateAccount ValidateAccount;
		TResendValidationCode ResendValidationCode;
		TRequestPasswordReset RequestPasswordReset;
		TNewPasswordRequest NewPasswordRequest;
		TSelectCharacter SelectCharacter;
		TLoginCharacterIndex LoginWithCharacter;
		TVoidParam ReturnToLogin;
		TCreateCharacter CreateCharacter;
		TSelectCharacter RequestDeleteCharacter;
		TIntStringF ConfirmDeleteCharacter;
		TIntStringF TransferCharacter;
	};

	typedef void(__stdcall* TSingleStringParam)(SingleStringParam*);
	typedef void(__stdcall* TSingleIntParam)(int);
	typedef void(__stdcall* TSingleBoolParam)(int);
	typedef void(__stdcall* TDoubleIntParam)(int, int);

	struct GameplayCallbacks
	{
		TSingleStringParam HandleConsoleMsg;
		TSingleStringParam ShowDialog;
		TSingleIntParam SelectInvSlot;
		TSingleIntParam UseInvSlot;
		TSingleIntParam SelectSpellSlot;
		TSingleIntParam UseSpellSlot;
		TSingleBoolParam UpdateFocus;
		TSingleBoolParam UpdateOpenDialog;
		TSingleStringParam OpenLink;
		TVoidParam ClickGold;
		TDoubleIntParam MoveInvItem;
		TSingleIntParam RequestAction;
		TSingleIntParam UseKey;
		TDoubleIntParam MoveSpellSlot;
		TSingleIntParam DeleteItem;
	};
}