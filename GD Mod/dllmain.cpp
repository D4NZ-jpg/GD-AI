// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <math.h>
#include <gd.h>
#include <support/zip_support/ZipUtils.h>
#include <support/base64.h>

const int cellSize = 20;
const int w = 20, h = 5;
std::map<std::pair<int, int>, int> lvl_map;

std::vector<std::string> split(std::string str, char delim)
{
    std::vector<std::string> out;
    size_t pos = 0;
    while ((pos = str.find(delim)) != std::string::npos)
    {
        out.push_back(str.substr(0, pos));
        str.erase(0, pos + 1);
    }
    out.push_back(str);
    return out;
}

std::string decode_level(const std::string& input) {
    unsigned char* levelString;
    unsigned char* levelStringFull;
    int levelStringSize = base64Decode((unsigned char*)(input.c_str()), input.size(), &levelString);
    int levelStringSizeDeflated = ZipUtils::ccInflateMemory(levelString, levelStringSize, &levelStringFull);

    std::string levelStringFullStd((char*)levelStringFull, levelStringSizeDeflated);

    delete levelString;
    delete levelStringFull;

    return levelStringFullStd;
}

std::map<std::pair<int,int>, int> build_level_map(std::string data)
{
    std::map<std::pair<int, int>, int> lvl;
    
    size_t pos = data.find(';');
    data.erase(0, pos + 1);
    for (std::string object : split(data, ';'))
    {
        std::vector<std::string> properties = split(object, ',');
        if (properties.size() < 6 || properties.size() % 2)
            continue;

        int x = -1;
        int y = -1;
        int id = -1;
        for (int i = 0; i < properties.size(); i += 2)
            switch (std::stoi(properties[i]))
            {
            case 1:
                id = std::stoi(properties[i+1]);
                break;
            case 2:
                x = std::roundf(std::stof(properties[i + 1]) / cellSize);
                break;
            case 3:
                y = std::roundf(std::stof(properties[i + 1]) / cellSize);
                break; 
            }

        if (x != -1 && y != -1 && id != -1)
            lvl[{x, y}] = id;
    }
    return lvl;
}

void visualize_map(std::pair<float,float> pos)
{
    int pos_x = std::roundf(pos.first / cellSize);
    int pos_y = std::roundf(pos.second / cellSize) - 4;
    std::string out = "";

    system("cls");
    for (int y = h; y > -h; y--)
    {
        for (int x = -3; x < w - 3; x++)
            if (lvl_map.count({ pos_x + x, pos_y + y }))
                out += std::to_string(lvl_map[{pos_x + x, pos_y + y}]);
            else
                out += '.';
        out += '\n';
    }

    out += "x: ";
    out += std::to_string(pos_x);
    out += ", y: ";
    out += std::to_string(pos_y);
    std::cout << out;
}

bool(__thiscall* PlayLayer_init)(gd::PlayLayer* self, gd::GJGameLevel* level);
bool __fastcall PlayLayer_init_H(gd::PlayLayer* self, void*, gd::GJGameLevel* level) {
    std::string lvl_string = decode_level(level->m_sLevelString);
    lvl_map = build_level_map(lvl_string);
    PlayLayer_init(self, level);
    return true;
}

bool(__thiscall* PlayLayer_update)(gd::PlayLayer* self, float dt);
bool __fastcall PlayLayer_update_H(gd::PlayLayer* self, void*, float dt)
{
    visualize_map({ self->m_pPlayer1->getPositionX(), self->m_pPlayer1->getPositionY() });
    PlayLayer_update(self, dt);
    return true;
}

DWORD WINAPI thread(void* hModule){
    // Init console 
    FILE* pFile = nullptr;
    AllocConsole();
    freopen_s(&pFile, "CONOUT$", "w", stdout);

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
