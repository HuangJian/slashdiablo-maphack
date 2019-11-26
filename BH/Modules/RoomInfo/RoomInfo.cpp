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

vector<tuple<string, bool, function<string()>>> RoomInfo::toggleList;

void RoomInfo::OnLoad() {
    RoomInfo::toggleList = {
        make_tuple("Layer Level No Toggle", true, []() {
            UnitAny* pUnit = D2CLIENT_GetPlayerUnit();
            auto levelNo = pUnit->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;
            return wstring_to_string(D2CLIENT_GetLevelName(levelNo));
        }),
        make_tuple("Layer Level No Toggle", true, []() {
            return RoomInfo::txtDifficulty;
        }),
        make_tuple("Area Level Toggle", true, []() {
            UnitAny* pUnit = D2CLIENT_GetPlayerUnit();
            auto levelNo = pUnit->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;
            int iLevelNo = levelNo & 0xffff; // map it to 1~65536
            auto it = RoomInfo::mapAreaLevels.find(iLevelNo);
            auto areaLevel = (it == mapAreaLevels.end()) ? "Unknown" : it->second;
            return "Area level: " + areaLevel;
        }),
        make_tuple("Server Ip Toggle", true, []() {
            return (*p_D2LAUNCH_BnData)->szGameIP;
        }),
        make_tuple("Clock Toggle", true, []() {
            auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
            return to_string(put_time(localtime(&now), "%X"));
        }),
        make_tuple("Game Time Toggle", true, []() {
            auto now = GetTickCount();
            int elapsedSeconds = (GetTickCount() - RoomInfo::gameTimer) / 1000;
            return string_format("%.2d:%.2d:%.2d", elapsedSeconds / 3600, (elapsedSeconds / 60) % 60, elapsedSeconds % 60);
        }),
    };
    for (auto const& item : toggleList) {
        string key = get<0>(item);
        BH::config->ReadToggle(key, "None", get<1>(item), Toggles[key]);
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
    int yOffset = 6;

    for (auto const& item : toggleList) {
        string key = get<0>(item);
        if (Toggles[key].state) {
            auto text = get<2>(item)();
            if (!text.empty()) {
                Texthook::Draw(*p_D2CLIENT_ScreenSizeX - 16, yOffset, Right, 0, Gold, text);
                yOffset += 16;
            }
        }
    }
}