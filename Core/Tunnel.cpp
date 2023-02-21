#include "Tunnel.h"
#include <string>
#include "SharedMemory/Events/EventHandler.hpp"
#include "Core/Logger.hpp"
namespace Babel
{
    namespace {

    }
    Tunnel::~Tunnel()
    {
        Event event;
        event.EventType = EventType::Close;
        event.Size = sizeof(event);
        mSyncData->GetApiMessenger().AddEvent((uint8_t*)&event, event.Size);
    }

    bool Tunnel::Initialize(int width, int height)
    {
        LOGGER->init("Logs/BabelAPI.log", "BabelAPI");
        mSyncData = std::make_unique<SyncData>(width, height, 4, 3);
        mSharedMemory = std::make_unique<SharedMemory>(mSyncData->GetTotalSize());
        if (!mSharedMemory->Create("Local\\TestMemShare2")) return false;
        mSyncData->GetSharedFileViews(*mSharedMemory);
        mEventHandler = std::make_unique<EventHandler>(*this, mSyncData->GetSlaveMessenger());
        std::string commandLine = "BabelSlave.exe ";
        commandLine += std::to_string(width) + " " + std::to_string(height);
        return mProcess.StartProcess(commandLine.c_str());
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
        if (!mDebugSharedMemory->Create("Local\\TestMemShare2Debug")) return false;
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
            const LoginInfoEvent& loginevt = static_cast<const LoginInfoEvent&>(eventData);
            LogInInfo loginInfo;
            loginInfo.User = loginevt.strData;
            loginInfo.Password = loginevt.strData + loginevt.UserSize;
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
            newAccountInfo.User = accountEvt.strData;
            offset += accountEvt.UserSize;
            newAccountInfo.UserLen = accountEvt.UserSize;
            newAccountInfo.Password = accountEvt.strData + offset;
            offset += accountEvt.PasswordSize;
            newAccountInfo.PasswordLen = accountEvt.PasswordSize;
            newAccountInfo.Name = accountEvt.strData + offset;
            offset += accountEvt.NameSize;
            newAccountInfo.NameLen = accountEvt.NameSize;
            newAccountInfo.Surname = accountEvt.strData + offset;
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
        eventInfo.localize = localize;
        eventInfo.Size = sizeof(eventInfo) + Babel::PrepareDynamicStrings(strInfo);
        GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
    }

}