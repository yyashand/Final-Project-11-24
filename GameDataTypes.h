#ifndef GAMEDATATYPES_H
#define GAMEDATATYPES_H

#include <string>
#include <vector>
#include <map> // Hanya jika ada struct yang mendeklarasikan map sebagai member

// Konstanta
const int PLAYER_DEFEND_HP_REGEN_AMOUNT = 15;
const int PLAYER_DEFEND_SP_REGEN_AMOUNT = 10;

enum class DefendResolution { NONE, EVADE_REGEN, HIT_THROUGH, SUCCESSFUL_BLOCK };

struct PlayerStats {
    int hp;
    int maxHp;
    int sp;
    int maxSp;
    int attack;
    int defense;
};

struct Enemy {
    std::string id;
    std::string name;
    int hp;
    int maxHp;
    int attack;
    int defense;
    std::string victorySceneId;
    std::string defeatEndingTitle;
};

struct SceneOption {
    std::string optionText;
    std::string nextSceneId;
    SceneOption(std::string text, std::string nextId); 
};

struct Scene {
    std::string sceneId;
    std::string description;
    std::vector<SceneOption> options;
    bool isEnding;
    std::string endingTitle;
    std::string enemyIdToFightOnEnter;

    Scene(std::string id = "", std::string desc = "", bool ending = false, std::string title = "", std::string enemyId = "");
    void addOption(const std::string& optionChar, const std::string& optionDesc, const std::string& nextSceneIdSuffixOrFullId, bool isFullId = false);
};

#endif // GAMEDATATYPES_H