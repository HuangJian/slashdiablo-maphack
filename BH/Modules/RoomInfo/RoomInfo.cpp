#include "RoomInfo.h"
#include "../../BH.h"
#include "../../D2Ptrs.h"
#include "../../D2Stubs.h"
#include "../../MPQReader.h"
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <iomanip> // put_time

using namespace Drawing;

map<std::string, Toggle> RoomInfo::Toggles;
map<int, string> RoomInfo::mapAreaLevels;
DWORD RoomInfo::gameTimer;
string RoomInfo::txtDifficulty;

void RoomInfo::OnLoad() {
    vector<string> toggleKeys = {
        "Layer Level No Toggle",
        "Area Level Toggle",
        "Server Ip Toggle",
        "Game Time Toggle",
        "Clock Toggle",
    };
    for (auto const& toggleKey : toggleKeys) {
        BH::config->ReadToggle(toggleKey, "None", true, Toggles[toggleKey]);
    }
}

void RoomInfo::extractAreaLevels(int difficulty) {
    RoomInfo::mapAreaLevels.clear();

    auto txt = MpqDataMap.find("levels");
    if (txt == MpqDataMap.end()) {
        BH::logger << "RoomInfo::extractAreaLevels: Failed to find levels.txt from mpq data." << endl;
        return;
    }
    bool isExpansion = (*p_D2CLIENT_ExpCharFlag) > 0;
    for (const auto& level : txt->second->data) {
        auto id = level.find("Id");
        if (id != level.end() && !id->second.empty()) {
            string key = string_format("MonLvl%d", difficulty + 1);
            if (isExpansion) {
                key += "Ex";
            }
            auto monsterLevel = level.find(key);
            if (monsterLevel != level.end()) {
                auto val = monsterLevel->second;
                if (val.empty()) {
                    val = "0";
                }
                RoomInfo::mapAreaLevels.insert(make_pair(std::stoi(id->second), val));
            }
        }
    }
}

void RoomInfo::OnGameJoin() {
    gameTimer = GetTickCount();

    int difficulty = D2CLIENT_GetDifficulty() & 0b11; // 0 -> Normal, 1 -> Nightmare, 2 -> Hell
    string difficulties[3] = { "Normal", "Nightmare", "Hell" };
    txtDifficulty = "Difficulty: " + difficulties[difficulty];

    extractAreaLevels(difficulty);
}

void RoomInfo::OnDraw() {
    int yOffset = 8;
    int xOffset = *p_D2CLIENT_ScreenSizeX - 16;

    UnitAny* pUnit = D2CLIENT_GetPlayerUnit();
    auto levelNo = pUnit->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;

    if (Toggles["Layer Level No Toggle"].state) {
        auto levelName = D2CLIENT_GetLevelName(levelNo);
        Texthook::Draw(xOffset, yOffset, Right, 0, Gold, levelName);
        yOffset += 16;

        Texthook::Draw(xOffset, yOffset, Right, 0, Gold, txtDifficulty);
        yOffset += 16;
    }

    if (Toggles["Area Level Toggle"].state) {
        int iLevelNo = levelNo & 0xffff; // map it to 1~65536
        auto it = mapAreaLevels.find(iLevelNo);
        auto areaLevel = (it == mapAreaLevels.end()) ? "Unknown" : it->second;
        Texthook::Draw(xOffset, yOffset, Right, 0, Gold, "Area level: " + areaLevel);
        yOffset += 16;
    }

    if (Toggles["Server Ip Toggle"].state) {
        string serverIp = (*p_D2LAUNCH_BnData)->szGameIP;
        if (!serverIp.empty()) {
            Texthook::Draw(xOffset, yOffset, Right, 0, Gold, serverIp);
            yOffset += 16;
        }
    }

    if (Toggles["Clock Toggle"].state) {
        auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        string worldTime = to_string(put_time(localtime(&now), "%X"));
        Texthook::Draw(xOffset, yOffset, Right, 0, Gold, worldTime);
        yOffset += 16;
    }

    if (Toggles["Game Time Toggle"].state) {
        auto now = GetTickCount();
        int elapsedSeconds = (GetTickCount() - gameTimer) / 1000;
        auto gameTime = string_format("%.2d:%.2d:%.2d", elapsedSeconds / 3600, (elapsedSeconds / 60) % 60, elapsedSeconds % 60);
        Texthook::Draw(xOffset, yOffset, Right, 0, Gold, gameTime);
        yOffset += 16;
    }
}