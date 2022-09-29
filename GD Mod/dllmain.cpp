// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"
#include <iostream>
#include <vector>
#include <gd.h>

bool(__thiscall* PlayLayer_init)(gd::PlayLayer* self, gd::GJGameLevel* level);
bool __fastcall PlayLayer_init_H(gd::PlayLayer* self, void*, gd::GJGameLevel* level) {
    std::cout << level->m_sLevelString << '\n';
    PlayLayer_init(self, level);
    return true;
}

bool(__thiscall* PlayLayer_update)(gd::PlayLayer* self, float dt);
bool __fastcall PlayLayer_update_H(gd::PlayLayer* self, void*, float dt)
{
    std::cout << '(' << self->m_pPlayer1->getPositionX() << ',' << self->m_pPlayer1->getPositionY() << ")\n";
    PlayLayer_update(self, dt);
    return true;
}


DWORD WINAPI thread(void* hModule) {
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

    FILE* pFile = nullptr;
    //Make a terminal for debug
    AllocConsole();
    freopen_s(&pFile, "CONOUT$", "w", stdout);

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

