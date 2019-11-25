#pragma once

#include "../../Config.h"
#include "../Module.h"

class RoomInfo : public Module {
    private:
        // have to declare them as "static" as they are initialized in "OnGameJoin" 
        // but accessed in "OnAutomapDraw" (different threads???)
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