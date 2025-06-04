// File: GameEngine.h

#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>       // Untuk std::unique_ptr
#include "GameDataTypes.h"
#include "Interfaces.h"    // Untuk Subject dan IEnemyFactory (jika IEnemyFactory adalah base)

// ---- FORWARD DECLARATIONS ----
class ChoiceLoggerObserver; 
// class IEnemyFactory; // Tidak perlu jika sudah diinclude dari Interfaces.h dan Interfaces.h tidak include GameEngine.h secara sirkular

class GameEngine : public Subject {
public:
    GameEngine();
    ~GameEngine(); // <--- DEKLARASI DESTRUCTOR DI SINI (PENTING!)

    void initializeGame();
    void run();
    void processNavigateChoice(char choice);

private:
    PlayerStats playerStats_;
    Enemy currentCombatEnemy_;
    bool inCombat_;
    DefendResolution currentDefendOutcome_; 

    std::map<std::string, Scene> scenes_;
    std::map<std::string, Enemy> enemyTemplates_; 
    std::unique_ptr<IEnemyFactory> enemyFactory_; 
    std::unique_ptr<ChoiceLoggerObserver> choiceObserver_; // <--- unique_ptr ini yang jadi masalah

    std::string currentSceneId_;
    bool running_;
    // std::unique_ptr<ChoiceLoggerObserver> choiceObserver_; // Sudah di atas
    std::set<std::string> allEndingTitles_;
    bool hasLegendarySword_;

    // Deklarasi metode private
    void loadScene(const Scene& scene);
    void internal_loadEnemy(const Enemy& enemy);
    // ... (sisa deklarasi metode private Anda)
    void promptPlayAgain();
    char getStandardInput();
    char getCombatInput();
    std::string replacePlaceholders(std::string text);
    void displayCurrentScene();
    void handleEnding(const std::string& endingTitleToRecord);
    void startCombat(const std::string& enemyId);
    void displayCombatInterface();
    void processPlayerCombatAction(char action);
    void processEnemyTurn();
    void checkCombatResult();
};

#endif // GAMEENGINE_H