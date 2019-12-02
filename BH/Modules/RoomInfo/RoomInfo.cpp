#include "RoomInfo.h"
#include "../../BH.h"
#include "../../D2Ptrs.h"
#include "../../MPQReader.h"
#include <ctime>   // localtime_s
#include <iomanip> // put_time

using namespace Drawing;

void RoomInfo::OnLoad() {
    Toggle layerLevelToggle{};
    features.assign({
        {"Layer Level No Toggle", layerLevelToggle, true, [this]() {
            auto levelNo = D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;
            return D2CLIENT_GetLevelName(levelNo);
        }},
        {"Layer Level No Toggle", layerLevelToggle, true, [this]() {
            return txtDifficulty;
        }},
        {"Area Level Toggle", Toggle(), true, [this]() {
            auto levelNo = D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;
            std::lock_guard<std::mutex> lock(this->mutexMap);
            auto levels = mapAreaLevels.find(static_cast<unsigned int>(levelNo));
            int difficulty = static_cast<int>(D2CLIENT_GetDifficulty()); // 0 -> Normal, 1 -> Nightmare, 2 -> Hell
            bool isExpansion = (*p_D2CLIENT_ExpCharFlag) > 0;
            if (isExpansion) {
                difficulty += 3;
            }
            auto areaLevel = (levels == mapAreaLevels.end()) ? L"Unknown" : to_wstring(levels->second[difficulty]);
            return L"Area level: " + areaLevel;
        }},
        {"Server Ip Toggle", Toggle(), true, []() {
            return string_to_wstring((*p_D2LAUNCH_BnData)->szGameIP);
        }},
        {"Clock Toggle", Toggle(), true, []() {
            time_t now = time(nullptr);
            struct tm timeInfo{};
            localtime_s(&timeInfo, &now);
            return to_wstring(put_time(&timeInfo, L"%X"));
        }},
        {"Game Time Toggle", Toggle(), true, [this]() {
            auto seconds = (GetTickCount() - gameTimer) / 1000;
            return wstring_format(L"%.2ld:%.2ld:%.2ld", seconds / 3600, (seconds / 60) % 60, seconds % 60);
        }},
    });
    RoomInfo::LoadConfig();
}

void RoomInfo::LoadConfig() {
    // it could be invoked in initializing module and reloading config.
    for (auto& item : features) {
        BH::config->ReadToggle(item.key, "None", item.defaultVal, item.toggle);
    }
}

void RoomInfo::MpqLoaded() {
    auto txt = MpqDataMap.find("levels");
    if (txt == MpqDataMap.end()) {
        BH::logger << "RoomInfo::MpqLoaded: Failed to find levels.txt from mpq data." << endl;
        return;
    }

    std::lock_guard<std::mutex> lock(this->mutexMap);
    vector<string> keys = {"MonLvl1", "MonLvl2", "MonLvl3", "MonLvl1Ex", "MonLvl2Ex", "MonLvl3Ex"};
    for (const auto& level : txt->second->data) {
        auto id = level.find("Id");
        if (id == level.end() || id->second.empty()) {
            continue;
        }

        int size = keys.size();
        vector<int> levels(size, 0);
        for (int i = 0; i < size; ++i) {
            auto monsterLevel = level.find(keys[i]);
            if (monsterLevel != level.end() && !monsterLevel->second.empty()) {
                levels[i] = std::stoi(monsterLevel->second);
            }
        }
        mapAreaLevels.insert(make_pair(std::stoi(id->second), levels));
    }
}

void RoomInfo::OnGameJoin() {
    gameTimer = GetTickCount();

    int difficulty = static_cast<int>(D2CLIENT_GetDifficulty()); // 0 -> Normal, 1 -> Nightmare, 2 -> Hell
    wstring difficulties[3] = { L"Normal", L"Nightmare", L"Hell" };
    txtDifficulty = L"Difficulty: " + difficulties[difficulty];
}

void RoomInfo::OnDraw() {
    int yOffset = 6;

    for (auto const& feature : features) {
        if (feature.toggle.state) {
            wstring text = feature.evalFunc();
            if (!text.empty()) {
                Texthook::Draw(*p_D2CLIENT_ScreenSizeX - 8, yOffset, Right, 0, Gold, text.c_str());
                yOffset += 16;
            }
        }
    }
}
