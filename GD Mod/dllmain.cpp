// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"
#include "zmq.hpp"
#include <iostream>
#include <vector>
#include <gd.h>

// ZeroMQ 
zmq::context_t context(1);
zmq::socket_t socket(context, zmq::socket_type::req);

bool(__thiscall* PlayLayer_init)(gd::PlayLayer* self, gd::GJGameLevel* level);
bool __fastcall PlayLayer_init_H(gd::PlayLayer* self, void*, gd::GJGameLevel* level) {
    //Build message
    auto msg = (std::string("init:") + level->m_sLevelString).c_str();

    //Send message
    zmq::message_t request(strlen(msg));
    memcpy(request.data(), msg, strlen(msg));
    socket.send(request, zmq::send_flags::none);

    //Get reply
    zmq::message_t reply;
    socket.recv(reply, zmq::recv_flags::none);

    PlayLayer_init(self, level);
    return true;
}

bool(__thiscall* PlayLayer_update)(gd::PlayLayer* self, float dt);
bool __fastcall PlayLayer_update_H(gd::PlayLayer* self, void*, float dt)
{
    //Build message
    float x = self->m_pPlayer1->getPositionX();
    float y = self->m_pPlayer1->getPositionY();
    auto msg = (std::string("update:") + std::to_string(x) + ',' + std::to_string(y)).c_str();

    //Send message
    zmq::message_t request(strlen(msg));
    memcpy(request.data(), msg, strlen(msg));
    socket.send(request, zmq::send_flags::none);

    //Get reply
    zmq::message_t reply;
    socket.recv(reply, zmq::recv_flags::none);
    std::cout << reply.to_string() << '\n';

    PlayLayer_update(self, dt);
    return true;
}

DWORD WINAPI thread(void* hModule){
    // Init console 
    FILE* pFile = nullptr;
    AllocConsole();
    freopen_s(&pFile, "CONOUT$", "w", stdout);

    // Connect ZeroMQ (Server must be already running)
    socket.connect("tcp://localhost:6969");

    //Hook functions
    if (MH_Initialize() != MH_OK)
        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);

    MH_CreateHook(
        reinterpret_cast<void*>(gd::base + 0x01FB780), // Adress where to hook
        reinterpret_cast<void*>(&PlayLayer_init_H),  // Function to be run
        reinterpret_cast<void**>(&PlayLayer_init)  // Trampoline function(?)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(gd::base + 0x2029C0),
        reinterpret_cast<void*>(&PlayLayer_update_H),
        reinterpret_cast<void**>(&PlayLayer_update)
    );

    MH_EnableHook(MH_ALL_HOOKS);
    return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0x1000, thread, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
