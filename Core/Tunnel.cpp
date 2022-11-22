#include "Tunnel.h"
#include <string>
#include "SharedMemory/Events/EventHandler.hpp"

namespace Babel
{
    Tunnel::~Tunnel()
    {
        Event event;
        event.EventType = EventType::Close;
        event.Size = sizeof(event);
        mSyncData->GetApiMessenger().AddEvent((uint8_t*)&event, event.Size);
    }

    bool Tunnel::Initialize(int width, int height)
    {
        mSyncData = std::make_unique<SyncData>(width, height, 4, 3);
        mSharedMemory = std::make_unique<SharedMemory>(mSyncData->GetTotalSize());
        if (!mSharedMemory->Create("Local\\TestMemShare2")) return false;
        mSyncData->GetSharedFileViews(*mSharedMemory);
        std::string commandLine = "BabelSlave.exe ";
        commandLine += std::to_string(width) + " " + std::to_string(height);
        return mProcess.StartProcess(commandLine.c_str());
    }

    void Tunnel::Terminate()
    {
        mSyncData.reset();
        mProcess.CloseProcess();
    }

}