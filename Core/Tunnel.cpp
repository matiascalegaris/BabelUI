#include "Tunnel.h"
#include <string>
#include "SharedMemory/Events/EventHandler.hpp"
#include "Core/Logger.hpp"
#include <windows.h>
#include <algorithm> 

namespace Babel
{
    namespace {
        std::string GetRandomString(size_t length)
        {
            auto randchar = []() -> char
            {
                const char charset[] =
                    "0123456789"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz";
                const size_t max_index = (sizeof(charset) - 1);
                return charset[rand() % max_index];
            };
            std::string str(length, 0);
            std::generate_n(str.begin(), length, randchar);
            return str;
        }
    }
    Tunnel::~Tunnel()
    {
        Event event;
        event.EventType = EventType::Close;
        event.Size = sizeof(event);
        mSyncData->GetApiMessenger().AddEvent((uint8_t*)&event, event.Size);
    }

    bool Tunnel::Initialize(const Settings& settings)
    {
        try {
            srand((unsigned)time(NULL) * GetCurrentProcessId());
            mSettings = settings;
            mSharedMemName = GetRandomString(8);
            LOGGER->init("Logs/BabelAPI.log", "BabelAPI");
            LOGGER->log("Initializing tunnel");
            mSyncData = std::make_unique<SyncData>(mSettings.Width, mSettings.Height, 4, 3);
            mSharedMemory = std::make_unique<SharedMemory>(mSyncData->GetTotalSize());
            if (!mSharedMemory->Create(("Local\\TestMemShare2" + mSharedMemName).c_str())) return false;
            mSyncData->GetSharedFileViews(*mSharedMemory);
            mEventHandler = std::make_unique<EventHandler>(*this, mSyncData->GetSlaveMessenger());
            std::string commandLine = "BabelSlave.exe ";
            commandLine += std::to_string(mSettings.Width) + " " + std::to_string(mSettings.Height)
                + " " + std::to_string(mSettings.Compmpressed) + " " + std::to_string(mSettings.EnableDebug)
                + " " + std::to_string(GetCurrentProcessId()) + " " + mSharedMemName;
            return mProcess.StartProcess(commandLine.c_str());
        }
        catch (...)
        {
            LOGGER->log("Failed to start the tunnel");
            return false;
        }
    }

    void Tunnel::Terminate()
    {
        mSyncData.reset();
        mProcess.CloseProcess();
    }

    bool Tunnel::InitializeDebugWindows(int width, int height)
    {
        if (mDebugSharedMemory)
        {
            return false;
        }
        mDebugSyncData = std::make_unique<SyncData>(width, height, 4, 3);
        mDebugSharedMemory = std::make_unique<SharedMemory>(mDebugSyncData->GetTotalSize());
        if (!mDebugSharedMemory->Create(("Local\\TestMemShare2Debug" + mSharedMemName).c_str())) return false;
        mDebugSyncData->GetSharedFileViews(*mDebugSharedMemory);

        Babel::WindowInfo windowData;
        windowData.Width = width;
        windowData.Height = height;
        windowData.EventType = Babel::EventType::EnableDebugWindow;
        windowData.Size = sizeof(Babel::WindowInfo);
        mSyncData->GetApiMessenger().AddEvent((uint8_t*)&windowData, windowData.Size);
        return true;
    }

    void Tunnel::HandleEvent(const Event& eventData)
    {
        switch (eventData.EventType)
        {
        case EventType::CloseClient:
            mVBCallbacks.CloseClient();
            break;
        case EventType::Login:
        {
            const LoginCredentialsEvent& loginevt = static_cast<const LoginCredentialsEvent&>(eventData);
            LogInInfo loginInfo;
            loginInfo.User = loginevt.StrData;
            loginInfo.Password = loginevt.StrData + loginevt.UserSize;
            loginInfo.UserLen = loginevt.UserSize;
            loginInfo.PasswordLen = loginevt.PasswordSize;
            loginInfo.StoreCredentials = loginevt.storeCredentials;
            mVBCallbacks.Login(&loginInfo);
        }
            break;
        case EventType::CreateAccount:
        {
            const NewAccountEvent& accountEvt = static_cast<const NewAccountEvent&>(eventData);
            NewAccountInfo newAccountInfo;
            int32_t offset = 0;
            newAccountInfo.User = accountEvt.StrData;
            offset += accountEvt.UserSize;
            newAccountInfo.UserLen = accountEvt.UserSize;
            newAccountInfo.Password = accountEvt.StrData + offset;
            offset += accountEvt.PasswordSize;
            newAccountInfo.PasswordLen = accountEvt.PasswordSize;
            newAccountInfo.Name = accountEvt.StrData + offset;
            offset += accountEvt.NameSize;
            newAccountInfo.NameLen = accountEvt.NameSize;
            newAccountInfo.Surname = accountEvt.StrData + offset;
            offset += accountEvt.SurnameSize;
            newAccountInfo.SurnameLen = accountEvt.SurnameSize;
            mVBCallbacks.CreateAccount(&newAccountInfo);
        }
        break;
        case EventType::ResendValidationCode:
        {
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(1);
            const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
            auto size = output - (char*)(&eventData);
            SingleStringParam strParam;
            strParam.Len = strInfo[0].Size;
            strParam.Str = strInfo[0].StartPos;
            mVBCallbacks.ResendValidationCode(&strParam);
        }
            break;
        case EventType::ValidateCode:
        {
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(2);
            const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
            auto size = output - (char*)(&eventData);
            std::string name(strInfo[0].StartPos, strInfo[0].Size);
            DoubleStringParam strParam;
            strParam.FirstLen = strInfo[0].Size;
            strParam.FirstStr = strInfo[0].StartPos;
            strParam.SecondLen = strInfo[1].Size;
            strParam.SecondStr = strInfo[1].StartPos;
            mVBCallbacks.ValidateAccount(&strParam);
        }
            break;
        case EventType::SetHost:
        {
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(2);
            const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
            auto size = output - (char*)(&eventData);
            std::string name(strInfo[0].StartPos, strInfo[0].Size);
            SingleStringParam strParam;
            strParam.Len = strInfo[0].Size;
            strParam.Str = strInfo[0].StartPos;
            mVBCallbacks.SetHost(&strParam);
        }
            break;
        case EventType::RequestPasswordReset:
        {
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(1);
            const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
            auto size = output - (char*)(&eventData);
            SingleStringParam strParam;
            strParam.Len = strInfo[0].Size;
            strParam.Str = strInfo[0].StartPos;
            mVBCallbacks.RequestPasswordReset(&strParam);
        }
        break;
        case EventType::NewPasswordRequest:
        {
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(3);
            const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
            auto size = output - (char*)(&eventData);
            TripleStringParam strParam;
            strParam.FirstLen = strInfo[0].Size;
            strParam.FirstStr = strInfo[0].StartPos;
            strParam.SecondLen = strInfo[1].Size;
            strParam.SecondStr = strInfo[1].StartPos;
            strParam.ThirdLen = strInfo[2].Size;
            strParam.ThirdStr = strInfo[2].StartPos;
            mVBCallbacks.NewPasswordRequest(&strParam);
        }
        break;
        case EventType::SelectCharacter:
        {
            const SelectCharacterEvent& selectEvt = static_cast<const SelectCharacterEvent&>(eventData);
            mVBCallbacks.SelectCharacter(selectEvt.CharIndex);
        }
        break;
        case EventType::LoginCharacter:
        {
            const SelectCharacterEvent& selectEvt = static_cast<const SelectCharacterEvent&>(eventData);
            mVBCallbacks.LoginWithCharacter(selectEvt.CharIndex);
        }
        break;
        case EventType::ReturnToLogin:
            mVBCallbacks.ReturnToLogin();
        break;
        case EventType::CreateCharacter:
        {
            const NewCharacterEvent& createChar = static_cast<const NewCharacterEvent&>(eventData);
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(1);
            const char* output = GetStringPtrInEvent((char*)(&createChar), sizeof(NewCharacterEvent), strInfo);
            NewCharacterInfo createCharacter;
            createCharacter.City = createChar.HomeCity;
            createCharacter.Class = createChar.Class;
            createCharacter.Gender = createChar.Gender;
            createCharacter.Head = createChar.Head;
            createCharacter.Race = createChar.Race;
            createCharacter.NameStr = strInfo[0].StartPos;
            createCharacter.NameLen = strInfo[0].Size;
            mVBCallbacks.CreateCharacter(&createCharacter);
        }
        break;
        case EventType::RequestDeleteCharacter:
        {
            const SelectCharacterEvent& selectEvt = static_cast<const SelectCharacterEvent&>(eventData);
            mVBCallbacks.RequestDeleteCharacter(selectEvt.CharIndex);
        }
        break;
        case EventType::ConfirmDeleteCharacter:
        {
            const SelectCharacterEvent& selectEvt = static_cast<const SelectCharacterEvent&>(eventData);
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(1);
            const char* output = GetStringPtrInEvent((char*)(&selectEvt), sizeof(SelectCharacterEvent), strInfo);
            SingleStringParam strParam;
            strParam.Len = strInfo[0].Size;
            strParam.Str = strInfo[0].StartPos;
            mVBCallbacks.ConfirmDeleteCharacter(selectEvt.CharIndex, &strParam);
        }
        break;
        case EventType::RequestTransferCharacter:
        {
            const SelectCharacterEvent& selectEvt = static_cast<const SelectCharacterEvent&>(eventData);
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(1);
            const char* output = GetStringPtrInEvent((char*)(&selectEvt), sizeof(SelectCharacterEvent), strInfo);
            SingleStringParam strParam;
            strParam.Len = strInfo[0].Size;
            strParam.Str = strInfo[0].StartPos;
            mVBCallbacks.TransferCharacter(selectEvt.CharIndex, &strParam);
        }
        break;
        case EventType::SendConsoleMsgToVB:
        {
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(1);
            const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
            auto size = output - (char*)(&eventData);
            SingleStringParam strParam;
            strParam.Len = strInfo[0].Size;
            strParam.Str = strInfo[0].StartPos;
            mGameplayVBcallbacks.HandleConsoleMsg(&strParam);
        }
        break;
        case EventType::ShowVBDialog:
        {
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(1);
            const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
            auto size = output - (char*)(&eventData);
            SingleStringParam strParam;
            strParam.Len = strInfo[0].Size;
            strParam.Str = strInfo[0].StartPos;
            mGameplayVBcallbacks.ShowDialog(&strParam);
        }
        break;
        case EventType::SelectInvSlot:
        {
            const SingleIntEvent& selectEvt = static_cast<const SingleIntEvent&>(eventData);
            mGameplayVBcallbacks.SelectInvSlot(selectEvt.Value + 1);
        }
        break;
        case EventType::UseInvSlot:
        {
            const SingleIntEvent& selectEvt = static_cast<const SingleIntEvent&>(eventData);
            mGameplayVBcallbacks.UseInvSlot(selectEvt.Value + 1);
        }
        break;
        case EventType::SelectSpellSlot:
        {
            const SingleIntEvent& selectEvt = static_cast<const SingleIntEvent&>(eventData);
            mGameplayVBcallbacks.SelectSpellSlot(selectEvt.Value + 1);
        }
        break;
        case EventType::UseSpellSlot:
        {
            const SingleIntEvent& selectEvt = static_cast<const SingleIntEvent&>(eventData);
            mGameplayVBcallbacks.UseSpellSlot(selectEvt.Value + 1);
        }
        break;
        case EventType::UpdateInputFocus:
        {
            const SingleBoolEvent& selectEvt = static_cast<const SingleBoolEvent&>(eventData);
            mGameplayVBcallbacks.UpdateFocus(selectEvt.Value);
        }
        break;
        case EventType::ClickLink:
        {
            std::vector<StringInBuffer> strInfo;
            strInfo.resize(1);
            const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
            auto size = output - (char*)(&eventData);
            SingleStringParam strParam;
            strParam.Len = strInfo[0].Size;
            strParam.Str = strInfo[0].StartPos;
            mGameplayVBcallbacks.OpenLink(&strParam);
            break;
        }
        case EventType::ClickGold:
        {
            mGameplayVBcallbacks.ClickGold();
            break;
        }
        case EventType::MoveInvSlot:
        {
            const DoubleIntEvent& slotsInfo = static_cast<const DoubleIntEvent&>(eventData);
            mGameplayVBcallbacks.MoveInvItem(slotsInfo.Value1 + 1, slotsInfo.Value2 + 1);
            break;
        }
        case EventType::RequestAction:
        {
            const SingleIntEvent& slotsInfo = static_cast<const SingleIntEvent&>(eventData);
            mGameplayVBcallbacks.RequestAction(slotsInfo.Value);
            break;
        }
        case EventType::UseKey:
        {
            const SingleIntEvent& slotsInfo = static_cast<const SingleIntEvent&>(eventData);
            mGameplayVBcallbacks.UseKey(slotsInfo.Value);
            break;
        }
        case EventType::MoveSpellSlot:
        {
            const DoubleIntEvent& slotsInfo = static_cast<const DoubleIntEvent&>(eventData);
            mGameplayVBcallbacks.MoveSpellSlot(slotsInfo.Value1 + 1, slotsInfo.Value2 + 1);
            break;
        }
        case EventType::DeleteItem:
        {
            const SingleIntEvent& evtData = static_cast<const SingleIntEvent&>(eventData);
            mGameplayVBcallbacks.DeleteItem(evtData.Value + 1);
            break;
        }
        case EventType::UpdateOpenDialog:
        {
            const SingleBoolEvent& evtData = static_cast<const SingleBoolEvent&>(eventData);
            mGameplayVBcallbacks.UpdateOpenDialog(evtData.Value);
            break;
        }
        }
    }

    void Tunnel::CheckIncommingMessages()
    {
        mEventHandler->HandleIncomingEvents();
    }

    void Tunnel::SetActiveScreen(const char* name)
    {
        Babel::Event eventInfo;
        eventInfo.EventType = Babel::EventType::SetActiveScreen;
        std::vector<StringInBuffer> strInfo(1);
        strInfo[0].StartPos = name;
        eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
        GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
    }

    void Tunnel::SetLoadingMessage(const char* message, bool localize)
    {
        Babel::LoadingMessage eventInfo;
        eventInfo.EventType = Babel::EventType::SetLoadingMessage;
        std::vector<Babel::StringInBuffer> strInfo(1);
        strInfo[0].StartPos = message;
        eventInfo.Localize = localize;
        eventInfo.Size = sizeof(eventInfo) + Babel::PrepareDynamicStrings(strInfo);
        GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
    }

}