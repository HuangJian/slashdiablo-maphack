#pragma once

#include "../../Config.h"
#include "../Module.h"
#include <tuple>
#include <functional>

class RoomInfo : public Module {
    private:
        // have to declare them as "static" as they are initialized in "OnGameJoin" 
        // but accessed in "OnAutomapDraw" (different threads???)
        static vector<tuple<string, bool, function<string()>>> toggleList;
        static map<string, Toggle> Toggles;
        static map<int, string> mapAreaLevels;
        static DWORD gameTimer;
        static string txtDifficulty;

        void extractAreaLevels(int difficulty);

    public:
        RoomInfo() : Module("Room Info") {};

        void OnLoad();
        void OnGameJoin();
        void OnDraw();
};