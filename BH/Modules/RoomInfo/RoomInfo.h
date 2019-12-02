#pragma once

#include "../../Config.h"
#include "../Module.h"
#include <functional>
#include <mutex>

struct RoomInfoFeature {
    string key;
    Toggle toggle;
    bool defaultVal;
    function<wstring()> evalFunc;
};

class RoomInfo : public Module {
private:
    // use a mutex here to ensure thread safe map accessing.
    mutex mutexMap;
    map<int, vector<int>> mapAreaLevels;

    vector<RoomInfoFeature> features;
    DWORD gameTimer;
    wstring txtDifficulty;
public:
    RoomInfo() : Module("Room Info") {};

    void OnLoad();

    void LoadConfig();
    void MpqLoaded();

    void OnGameJoin();
    void OnDraw();
};