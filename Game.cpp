#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <memory>
#include <limits>
#include <cctype>
#include <optional>
#include <cstdlib> 
#include <ctime>   
#include <stdexcept> 

// Forward declarations
class GameEngine;

//--------------------------------------------------------------------------------------------------
// Stats and Combat Definitions (Pindahkan definisi struct Enemy ke atas)
//--------------------------------------------------------------------------------------------------
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

// Konstanta
const int PLAYER_DEFEND_HP_REGEN_AMOUNT = 15;
const int PLAYER_DEFEND_SP_REGEN_AMOUNT = 10;

enum class DefendResolution { NONE, EVADE_REGEN, HIT_THROUGH, SUCCESSFUL_BLOCK };

//--------------------------------------------------------------------------------------------------
// Factory Pattern for Enemies
//--------------------------------------------------------------------------------------------------
class IEnemyFactory {
public:
    virtual ~IEnemyFactory() = default;
    virtual Enemy createEnemy(const std::string& enemyId) = 0; 
};

class ConfigurableEnemyFactory : public IEnemyFactory {
private:
    std::map<std::string, Enemy> enemyBlueprints_; 
public:
    ConfigurableEnemyFactory(const std::map<std::string, Enemy>& blueprints)
        : enemyBlueprints_(blueprints) {}

    Enemy createEnemy(const std::string& enemyId) override { // struct Enemy sekarang sudah terdefinisi lengkap
        if (enemyBlueprints_.count(enemyId)) {
            return enemyBlueprints_.at(enemyId); 
        }
        throw std::runtime_error("Enemy blueprint not found in factory: " + enemyId);
    }
};

//--------------------------------------------------------------------------------------------------
// Observer Pattern (Definisi lengkap dan hanya sekali)
//--------------------------------------------------------------------------------------------------
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& choiceMade) = 0;
};

class ChoiceLoggerObserver : public Observer {
public:
    void update(const std::string& choiceMade) override {
        std::cout << "\n--- [Notifikasi Observer] --- \nPilihan '" << choiceMade << "' (Non-Combat) telah dipilih.\n---------------------------\n";
    }
};

class Subject {
public:
    virtual ~Subject() = default;
    void attach(Observer* observer) {
        observers_.push_back(observer);
    }
    void detach(Observer* observer) {
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        if (it != observers_.end()) {
            observers_.erase(it);
        }
    }
    void notify(const std::string& choiceMade) {
        for (Observer* observer : observers_) {
            observer->update(choiceMade);
        }
    }
private:
    std::vector<Observer*> observers_;
};

//--------------------------------------------------------------------------------------------------
// Scene Definition
//--------------------------------------------------------------------------------------------------
struct SceneOption {
    std::string optionText;
    std::string nextSceneId;
    SceneOption(std::string text, std::string nextId) : optionText(std::move(text)), nextSceneId(std::move(nextId)) {}
};

struct Scene {
    std::string sceneId;
    std::string description;
    std::vector<SceneOption> options;
    bool isEnding;
    std::string endingTitle;
    std::string enemyIdToFightOnEnter;

    Scene(std::string id = "", std::string desc = "", bool ending = false, std::string title = "", std::string enemyId = "")
        : sceneId(std::move(id)), description(std::move(desc)), isEnding(ending), endingTitle(std::move(title)), enemyIdToFightOnEnter(std::move(enemyId)) {}

    void addOption(const std::string& optionChar, const std::string& optionDesc, const std::string& nextSceneIdSuffixOrFullId, bool isFullId = false) {
        if (isFullId) {
            options.emplace_back(optionChar + ". " + optionDesc, nextSceneIdSuffixOrFullId);
        } else {
            options.emplace_back(optionChar + ". " + optionDesc, this->sceneId + nextSceneIdSuffixOrFullId);
        }
    }
};

//--------------------------------------------------------------------------------------------------
// Singleton Pattern - Ending Tracker
//--------------------------------------------------------------------------------------------------
class EndingTracker {
public:
    static EndingTracker& getInstance() { static EndingTracker instance; return instance; }
    EndingTracker(const EndingTracker&) = delete; void operator=(const EndingTracker&) = delete;
    void recordEnding(const std::string& endingTitle) { if (!endingTitle.empty()) unlockedEndings_.insert(endingTitle); }
    int getUnlockedEndingsCount() const { return unlockedEndings_.size(); }
    void displayUnlockedEndings() const {
        if (unlockedEndings_.empty()) { std::cout << "Belum ada ending yang terbuka." << std::endl; }
        else {
            std::cout << "Ending yang telah terbuka (" << unlockedEndings_.size() << "/" << totalPossibleEndings_ << "):\n";
            for (const auto& title : unlockedEndings_) std::cout << "- " << title << std::endl;
        }
    }
    void setTotalPossibleEndings(int total) { totalPossibleEndings_ = total; }
    int getTotalPossibleEndings() const { return totalPossibleEndings_; }
private:
    EndingTracker() : totalPossibleEndings_(0) {}
    std::set<std::string> unlockedEndings_; int totalPossibleEndings_;
};

//--------------------------------------------------------------------------------------------------
// Command Pattern (Definisi lengkap dan hanya sekali)
//--------------------------------------------------------------------------------------------------
class Command {
public:
    virtual ~Command() = default; 
    virtual void execute() = 0;
};

class NavigateCommand : public Command {
public:
    NavigateCommand(GameEngine& gameEngine, char choice) : gameEngine_(gameEngine), choice_(choice) {}
    void execute() override; // Deklarasi di sini, definisi setelah GameEngine
private:
    GameEngine& gameEngine_; 
    char choice_;
};

//--------------------------------------------------------------------------------------------------
// Game Engine
//--------------------------------------------------------------------------------------------------
class GameEngine : public Subject { // Pastikan inheritance dari Subject
public:
    GameEngine() : 
        currentSceneId_(""), 
        running_(true), 
        hasLegendarySword_(false), 
        inCombat_(false),
        currentDefendOutcome_(DefendResolution::NONE)
    {
        srand(static_cast<unsigned int>(time(NULL))); 
        choiceObserver_ = std::make_unique<ChoiceLoggerObserver>();
        this->attach(choiceObserver_.get()); // 'this->' bersifat opsional, tapi jelas
    }

    void loadScene(const Scene& scene) { // Definisi loadScene
        scenes_[scene.sceneId] = scene;
        if (scene.isEnding && !scene.endingTitle.empty()) {
            allEndingTitles_.insert(scene.endingTitle);
        }
    }

    void initializeGame() {
        hasLegendarySword_ = false;
        inCombat_ = false;
        currentDefendOutcome_ = DefendResolution::NONE;
        scenes_.clear();
        allEndingTitles_.clear();
        
        enemyTemplates_.clear(); 
        internal_loadEnemy({"GOROK_01", "Gorok si Penghancur", 80, 80, 20, 8, "1AAA", "ENDING 2: Gugur di Tangan Letnan Gorok"});
        internal_loadEnemy({"SHADOW_GUARD", "Penjaga Bayangan", 70, 70, 18, 6, "LOKI_ENTRANCE_AFTER_GUARD", "ENDING 8: Dikalahkan Penjaga Bayangan"});
        internal_loadEnemy({"TWIN_BLADE_1", "Assassin Twin Blade (1/2)", 90, 90, 22, 7, "FIGHT_TWIN_2_SCENE", "ENDING 9: Tewas oleh Twin Blade Duo"});
        internal_loadEnemy({"TWIN_BLADE_2", "Assassin Twin Blade (2/2)", 90, 90, 24, 7, "POST_TWIN_BLADES_VICTORY", "ENDING 9: Tewas oleh Twin Blade Duo"}); // Menggunakan ending yang sama
        internal_loadEnemy({"LOKI_01", "Raja Iblis Loki", 200, 200, 30, 10, "LOKI_VICTORY", "ENDING 7: Dikalahkan Raja Iblis Loki"});

        enemyFactory_ = std::make_unique<ConfigurableEnemyFactory>(enemyTemplates_);

        playerStats_ = {150, 150, 120, 120, 15, 5}; 

        Scene scene1("1", "Heismay, (HP: {PLAYER_HP}, SP: {PLAYER_SP}) ksatria muda Ordo Charadrius, di depan Benteng Bayangan. Misi: Kalahkan Raja Iblis Loki. Langkah pertamamu?", false);
        scene1.addOption("A", "Masuk gerbang utama.", "A");
        scene1.addOption("B", "Cari jalan rahasia.", "B");
        loadScene(scene1); // Sekarang loadScene dikenali

        Scene scene1A("1A", "Gerbang utama terbuka. Gorok si Penghancur menghadang! 'Mangsa baru!'", false, "", "GOROK_01");
        loadScene(scene1A);

        Scene scene1B("1B", "Kau menemukan terowongan tua tersembunyi. Udara pengap keluar.", false);
        scene1B.addOption("A", "Masuk ke terowongan.", "A"); 
        scene1B.addOption("B", "Kembali ke gerbang utama.", "1A", true);
        loadScene(scene1B);
        
        Scene scene1AAA("1AAA", "Gorok terkapar kalah! Namun sebelum kau melangkah jauh, seorang Penjaga Bayangan lain muncul dari kegelapan! (HP: {PLAYER_HP}, SP: {PLAYER_SP})", false, "", "SHADOW_GUARD");
        loadScene(scene1AAA);
        
        Scene sceneLokiEntranceAfterGuard("LOKI_ENTRANCE_AFTER_GUARD", "Penjaga Bayangan itu pun tumbang. Jalan menuju jantung benteng kini lebih aman. (HP: {PLAYER_HP}, SP: {PLAYER_SP})", false);
        sceneLokiEntranceAfterGuard.addOption("A", "Lanjutkan ke ruang tahta Loki!", "LOKI_ENTRANCE", true);
        loadScene(sceneLokiEntranceAfterGuard);

        Scene scene1AB("1AB", "Mencoba menyelinap, tapi Gorok waspada. 'Mau kemana?' Gelap.", true, "ENDING 1: Terlalu Ceroboh");
        loadScene(scene1AB);
        
        Scene scene1BA("1BA", "Terowongan membawamu ke ruang rahasia. Ada peti tua berukir.", false);
        scene1BA.addOption("A", "Buka peti.", "A"); 
        scene1BA.addOption("B", "Abaikan peti, cari jalan ke Loki.", "B"); 
        loadScene(scene1BA);

        Scene scene1BAA("1BAA", "Di dalam peti, Pedang Cahaya Solaris! Kekuatanmu berlipat ganda! Tiba-tiba, dua bayangan bergerak cepat ke arahmu!", false);
        scene1BAA.addOption("A", "Hadapi mereka!", "FIGHT_TWIN_1_SCENE", true); 
        loadScene(scene1BAA);

        Scene sceneFightTwin1("FIGHT_TWIN_1_SCENE", "Assassin pertama dari duo Twin Blade menyerang dengan lincah!", false, "", "TWIN_BLADE_1");
        loadScene(sceneFightTwin1);

        Scene sceneFightTwin2("FIGHT_TWIN_2_SCENE", "Assassin pertama tumbang! Tanpa jeda, pasangannya yang lebih kuat langsung menyerang!", false, "", "TWIN_BLADE_2");
        loadScene(sceneFightTwin2);

        Scene scenePostTwins("POST_TWIN_BLADES_VICTORY", "Kedua assassin Twin Blade telah kau kalahkan! Jalanmu menuju Loki kini lebih aman. (HP: {PLAYER_HP}, SP: {PLAYER_SP})", false);
        scenePostTwins.addOption("A", "Lanjutkan ke ruang tahta Loki!", "LOKI_ENTRANCE", true);
        loadScene(scenePostTwins);

        Scene scene1BAB("1BAB", "Kau mengabaikan peti. Sebuah tangga sempit membawamu naik.", false);
        scene1BAB.addOption("A", "Naik tangga menuju takdir!", "LOKI_ENTRANCE", true); 
        loadScene(scene1BAB);

        Scene sceneLokiEntrance("LOKI_ENTRANCE", "Pintu besar ruang tahta Loki. Auranya gelap menekan. (HP: {PLAYER_HP}, SP: {PLAYER_SP})", false);
        sceneLokiEntrance.addOption("A", "Masuk dan hadapi Raja Iblis!", "LOKI_CONFRONT", true);
        loadScene(sceneLokiEntrance);

        Scene sceneLokiConfront("LOKI_CONFRONT", "", false, "", "LOKI_01");
        loadScene(sceneLokiConfront);

        Scene sceneLokiVictory("LOKI_VICTORY", "Raja Iblis Loki musnah! Cahaya kembali. Heismay sang pahlawan Charadrius!", true, "ENDING 3: Pahlawan Cahaya Charadrius");
        loadScene(sceneLokiVictory);
        
        // internal_loadEnemy sudah mencatat ending defeat
        allEndingTitles_.insert("ENDING 1: Terlalu Ceroboh");
        allEndingTitles_.insert("ENDING 3: Pahlawan Cahaya Charadrius");

        EndingTracker::getInstance().setTotalPossibleEndings(allEndingTitles_.size());
        currentSceneId_ = "1";
        if (scenes_.find(currentSceneId_) == scenes_.end()) {
            std::cerr << "Error: Scene awal '" << currentSceneId_ << "' tidak ditemukan!" << std::endl;
            running_ = false;
        }
    }

    void run() { 
        if (!running_) return;
        while (running_) {
            if (playerStats_.hp <= 0 && !inCombat_) { running_ = false; break; }
            if (scenes_.find(currentSceneId_) == scenes_.end()) {
                std::cerr << "Error: currentSceneId_ ('" << currentSceneId_ << "') tidak valid!" << std::endl;
                running_ = false; break;
            }
            const auto& currentScene = scenes_.at(currentSceneId_);

            if (inCombat_) {
                displayCombatInterface();
                char combatActionInput = getCombatInput();
                processPlayerCombatAction(combatActionInput);
                if (currentCombatEnemy_.hp > 0 && playerStats_.hp > 0) { processEnemyTurn(); }
                checkCombatResult();
            } else {
                displayCurrentScene(); 
                if (currentScene.isEnding) {
                    handleEnding(currentScene.endingTitle); promptPlayAgain();
                } else if (!currentScene.enemyIdToFightOnEnter.empty()) {
                    std::cout << "\n--- PERTARUNGAN DIMULAI! ---" << std::endl;
                    startCombat(currentScene.enemyIdToFightOnEnter);
                } else {
                    char navChoice = getStandardInput();
                    // make_unique sekarang seharusnya bekerja karena NavigateCommand terdefinisi dengan benar
                    auto command = std::make_unique<NavigateCommand>(*this, navChoice); 
                    command->execute(); // execute() pada NavigateCommand juga seharusnya dikenali
                }
            }
        }
        std::cout << "\nTerima kasih telah bermain!" << std::endl;
        EndingTracker::getInstance().displayUnlockedEndings();
    }
    
    void processNavigateChoice(char choice) { 
        if (inCombat_ || scenes_.find(currentSceneId_) == scenes_.end()) return;
        const auto& currentScene = scenes_.at(currentSceneId_);
        std::string targetNextSceneId = "";
        char upperChoice = static_cast<char>(toupper(static_cast<unsigned char>(choice)));

        for (const auto& opt : currentScene.options) {
            if (!opt.optionText.empty() && static_cast<char>(toupper(static_cast<unsigned char>(opt.optionText[0]))) == upperChoice) {
                targetNextSceneId = opt.nextSceneId; break;
            }
        }

        if (!targetNextSceneId.empty() && scenes_.count(targetNextSceneId)) {
            if (currentSceneId_ == "1BA" && upperChoice == 'A' && targetNextSceneId == "1BAA") {
                if (!hasLegendarySword_) { 
                    hasLegendarySword_ = true; 
                    int oldAttack = playerStats_.attack;
                    playerStats_.attack *= 2; 
                    std::cout << "\n*** Kamu mendapatkan Pedang Cahaya Solaris! Kekuatan seranganmu berlipat ganda dari " 
                              << oldAttack << " menjadi " << playerStats_.attack << "! ***\n";
                }
            }
            currentSceneId_ = targetNextSceneId;
            this->notify(std::string(1, upperChoice)); // notify() adalah member dari Subject (base class)
        } else {
            std::cout << "Pilihan tidak valid. Coba lagi." << std::endl;
        }
    }

private:
    PlayerStats playerStats_;
    Enemy currentCombatEnemy_;
    bool inCombat_;
    DefendResolution currentDefendOutcome_; 

    std::map<std::string, Scene> scenes_;
    std::map<std::string, Enemy> enemyTemplates_; 
    std::unique_ptr<IEnemyFactory> enemyFactory_; 

    std::string currentSceneId_;
    bool running_;
    std::unique_ptr<ChoiceLoggerObserver> choiceObserver_;
    std::set<std::string> allEndingTitles_;
    bool hasLegendarySword_;

    void internal_loadEnemy(const Enemy& enemy) {
        enemyTemplates_[enemy.id] = enemy;
        if (!enemy.defeatEndingTitle.empty()) {
            allEndingTitles_.insert(enemy.defeatEndingTitle);
        }
    }

    void promptPlayAgain() { 
        char playAgainChoice;
        std::cout << "\nApakah Anda ingin bermain lagi? (y/n): ";
        std::cin >> playAgainChoice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (tolower(playAgainChoice) == 'y') {
            std::cout << "\nMemulai permainan baru...\n";
            initializeGame(); 
            EndingTracker::getInstance().displayUnlockedEndings(); 
        } else {
            running_ = false;
        }
    }
    char getStandardInput() { 
        std::cout << "Apa pilihanmu: -> ";
        char choiceInput;
        std::cin >> choiceInput;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return choiceInput;
    }
    char getCombatInput() { 
        std::cout << "Aksi Bertarung: -> ";
        char choiceInput;
        std::cin >> choiceInput;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return static_cast<char>(toupper(static_cast<unsigned char>(choiceInput)));
    }
    
    std::string replacePlaceholders(std::string text) { 
        size_t pos = text.find("{PLAYER_HP}");
        while (pos != std::string::npos) {
            text.replace(pos, std::string("{PLAYER_HP}").length(), std::to_string(playerStats_.hp) + "/" + std::to_string(playerStats_.maxHp));
            pos = text.find("{PLAYER_HP}", pos + 1);
        }
        pos = text.find("{PLAYER_SP}");
        while (pos != std::string::npos) {
            text.replace(pos, std::string("{PLAYER_SP}").length(), std::to_string(playerStats_.sp) + "/" + std::to_string(playerStats_.maxSp));
            pos = text.find("{PLAYER_SP}", pos + 1);
        }
        return text;
    }

    void displayCurrentScene() {
        if (!scenes_.count(currentSceneId_)) {
            std::cout << "Error: Scene '" << currentSceneId_ << "' tidak ditemukan." << std::endl;
            running_ = false; return;
        }
        Scene sceneToDisplay = scenes_.at(currentSceneId_); 
        std::string finalDescription = sceneToDisplay.description;
        if (currentSceneId_ == "LOKI_CONFRONT") {
            if (hasLegendarySword_) {
                finalDescription = "Kau akhirnya sampai di ruang tahta. Loki duduk dengan angkuh. Matanya sedikit terbelalak melihat kilau Solaris di tanganmu. 'Pedang itu...! Jadi kau pewaris Charadrius yang sebenarnya?'";
            } else {
                finalDescription = "Kau akhirnya sampai di ruang tahta. Loki duduk dengan angkuh. 'Ksatria Charadrius lain yang datang untuk mati? Menyedihkan.'";
            }
        }
        finalDescription = replacePlaceholders(finalDescription);
        std::cout << "\n\n==================================================" << std::endl;
        std::cout << finalDescription << std::endl;
        std::cout << "==================================================" << std::endl;
        if (sceneToDisplay.isEnding) { /* ... */ } 
        else if (!sceneToDisplay.enemyIdToFightOnEnter.empty()){ std::cout << "Bersiap untuk bertarung!" << std::endl; }
        else {
            for (const auto& opt : sceneToDisplay.options) {
                std::cout << replacePlaceholders(opt.optionText) << std::endl;
            }
        }
    }
    void handleEnding(const std::string& endingTitleToRecord) { 
        if (!endingTitleToRecord.empty()) {
             EndingTracker::getInstance().recordEnding(endingTitleToRecord);
        }
        std::cout << "\n--- Kamu telah mencapai sebuah akhir! ---" << std::endl;
    }

    void startCombat(const std::string& enemyId) { 
        if (!enemyFactory_) { 
             std::cerr << "Error: Enemy Factory belum diinisialisasi." << std::endl;
             return;
        }
        try {
            currentCombatEnemy_ = enemyFactory_->createEnemy(enemyId); 
            inCombat_ = true;
            currentDefendOutcome_ = DefendResolution::NONE; 
            
            if (currentCombatEnemy_.id == "LOKI_01" && hasLegendarySword_) {
                std::cout << "\nPedang Cahaya Solaris bersinar terang melawan kegelapan Loki!" << std::endl;
            }
            std::cout << "\n*** Melawan: " << currentCombatEnemy_.name << "! ***" << std::endl;

        } catch (const std::runtime_error& e) {
            std::cerr << "Error memulai pertarungan: " << e.what() << std::endl;
            running_ = false; 
        }
    }

    void displayCombatInterface() { 
        std::cout << "\n--- MODE BERTARUNG ---" << std::endl;
        std::cout << "Heismay: HP " << playerStats_.hp << "/" << playerStats_.maxHp 
                  << " | SP " << playerStats_.sp << "/" << playerStats_.maxSp << std::endl;
        std::cout << currentCombatEnemy_.name << ": HP " << currentCombatEnemy_.hp << "/" << currentCombatEnemy_.maxHp << std::endl;
        std::cout << "----------------------" << std::endl;
        std::cout << "A. Serangan Biasa (0 SP)" << std::endl;
        std::cout << "B. Rising Slash   (15 SP) - Fisik kecil, 3-5x hit" << std::endl;
        std::cout << "C. Sword Dance    (40 SP) - Fisik berat, 8x hit" << std::endl;
        std::cout << "D. Shining Arrow  (30 SP) - Sihir cahaya, 4-8x hit (AoE)" << std::endl;
        std::cout << "E. Samsara        (60 SP) - Sihir cahaya besar, 1x hit" << std::endl;
        std::cout << "F. Bertahan             - Peluang menghindar & regenerasi HP/SP, blok, atau gagal" << std::endl;
    }

    void processPlayerCombatAction(char action) { 
        int spCost = 0;
        currentDefendOutcome_ = DefendResolution::NONE;

        switch (action) {
            case 'A': 
                std::cout << "Heismay melakukan serangan biasa!" << std::endl;
                {
                    int damageDealt = std::max(1, playerStats_.attack - currentCombatEnemy_.defense);
                    currentCombatEnemy_.hp -= damageDealt; currentCombatEnemy_.hp = std::max(0, currentCombatEnemy_.hp);
                    std::cout << "Memberikan " << damageDealt << " damage fisik." << std::endl;
                }
                break;
            case 'B': 
                spCost = 15;
                if (playerStats_.sp >= spCost) {
                    playerStats_.sp -= spCost; std::cout << "Heismay menggunakan Rising Slash! (SP Sisa: " << playerStats_.sp << ")" << std::endl;
                    int roll_hits = rand() % 100; int hits = (roll_hits < 30) ? 5 : (roll_hits < 90) ? 4 : 3;
                    int totalDamage = 0; float damagePerHitMultiplier = 0.4f;
                    std::cout << "Menyerang " << hits << " kali:" << std::endl;
                    for (int i = 0; i < hits; ++i) {
                        int damageThisHit = std::max(1, static_cast<int>(playerStats_.attack * damagePerHitMultiplier) - currentCombatEnemy_.defense);
                        currentCombatEnemy_.hp -= damageThisHit; totalDamage += damageThisHit;
                        std::cout << "  Hit " << (i + 1) << ": " << damageThisHit << " damage." << std::endl;
                    }
                    currentCombatEnemy_.hp = std::max(0, currentCombatEnemy_.hp);
                    std::cout << "Total damage Rising Slash: " << totalDamage << " fisik." << std::endl;
                } else { std::cout << "SP tidak cukup untuk Rising Slash!" << std::endl; }
                break;
            case 'C': 
                spCost = 40;
                if (playerStats_.sp >= spCost) {
                    playerStats_.sp -= spCost; std::cout << "Heismay menggunakan Sword Dance! (SP Sisa: " << playerStats_.sp << ")" << std::endl;
                    int hits = 8; int totalDamage = 0; float damagePerHitMultiplier = 0.8f;
                     std::cout << "Menyerang " << hits << " kali:" << std::endl;
                    for (int i = 0; i < hits; ++i) {
                        int damageThisHit = std::max(1, static_cast<int>(playerStats_.attack * damagePerHitMultiplier) - currentCombatEnemy_.defense);
                        currentCombatEnemy_.hp -= damageThisHit; totalDamage += damageThisHit;
                        std::cout << "  Hit " << (i + 1) << ": " << damageThisHit << " damage." << std::endl;
                    }
                    currentCombatEnemy_.hp = std::max(0, currentCombatEnemy_.hp);
                    std::cout << "Total damage Sword Dance: " << totalDamage << " fisik." << std::endl;
                } else { std::cout << "SP tidak cukup untuk Sword Dance!" << std::endl; }
                break;
            case 'D': 
                spCost = 30;
                if (playerStats_.sp >= spCost) {
                    playerStats_.sp -= spCost; std::cout << "Heismay merapal Shining Arrow! (SP Sisa: " << playerStats_.sp << ")" << std::endl;
                    int hits = (rand() % 5) + 4; int totalDamage = 0; float damagePerHitMultiplier = 0.6f;
                    std::cout << "Panah cahaya menghujani musuh " << hits << " kali:" << std::endl;
                    for (int i = 0; i < hits; ++i) {
                        int damageThisHit = std::max(1, static_cast<int>(playerStats_.attack * damagePerHitMultiplier) - currentCombatEnemy_.defense);
                        currentCombatEnemy_.hp -= damageThisHit; totalDamage += damageThisHit;
                        std::cout << "  Panah " << (i + 1) << ": " << damageThisHit << " damage cahaya." << std::endl;
                    }
                    currentCombatEnemy_.hp = std::max(0, currentCombatEnemy_.hp);
                    std::cout << "Total damage Shining Arrow: " << totalDamage << " cahaya." << std::endl;
                } else { std::cout << "SP tidak cukup untuk Shining Arrow!" << std::endl; }
                break;
            case 'E': 
                spCost = 60;
                if (playerStats_.sp >= spCost) {
                    playerStats_.sp -= spCost; std::cout << "Heismay melepaskan kekuatan Samsara! (SP Sisa: " << playerStats_.sp << ")" << std::endl;
                    float damageMultiplier = 2.5f;
                    int damageDealt = std::max(1, static_cast<int>(playerStats_.attack * damageMultiplier) - currentCombatEnemy_.defense);
                    currentCombatEnemy_.hp -= damageDealt; currentCombatEnemy_.hp = std::max(0, currentCombatEnemy_.hp);
                    std::cout << "Samsara memberikan " << damageDealt << " damage cahaya besar." << std::endl;
                } else { std::cout << "SP tidak cukup untuk Samsara!" << std::endl; }
                break;
            case 'F': 
                std::cout << "Heismay mengambil posisi bertahan..." << std::endl;
                { 
                    int roll = rand() % 100; 
                    if (roll < 50) { currentDefendOutcome_ = DefendResolution::EVADE_REGEN; } 
                    else if (roll < 65) { currentDefendOutcome_ = DefendResolution::HIT_THROUGH; } 
                    else { currentDefendOutcome_ = DefendResolution::SUCCESSFUL_BLOCK; }
                }
                break;
            default:
                std::cout << "Aksi tidak dikenal. Heismay ragu-ragu dan kehilangan giliran." << std::endl;
                break;
        }
    }

    void processEnemyTurn() { 
        std::cout << currentCombatEnemy_.name << " menyerang Heismay!" << std::endl;
        int damageTaken = 0; 

        switch (currentDefendOutcome_) {
            case DefendResolution::EVADE_REGEN:
                playerStats_.hp = std::min(playerStats_.maxHp, playerStats_.hp + PLAYER_DEFEND_HP_REGEN_AMOUNT);
                playerStats_.sp = std::min(playerStats_.maxSp, playerStats_.sp + PLAYER_DEFEND_SP_REGEN_AMOUNT);
                std::cout << "Serangan musuh meleset! Heismay menghindar, memulihkan " 
                          << PLAYER_DEFEND_HP_REGEN_AMOUNT << " HP dan " 
                          << PLAYER_DEFEND_SP_REGEN_AMOUNT << " SP!" << std::endl;
                break;
            case DefendResolution::HIT_THROUGH:
                damageTaken = std::max(1, currentCombatEnemy_.attack - playerStats_.defense);
                playerStats_.hp -= damageTaken;
                std::cout << "Heismay mencoba bertahan, tapi serangan " << currentCombatEnemy_.name 
                          << " berhasil menembus, memberikan " << damageTaken << " damage!" << std::endl;
                break;
            case DefendResolution::SUCCESSFUL_BLOCK:
                std::cout << "Heismay berhasil menahan serangan " << currentCombatEnemy_.name << "!" << std::endl;
                break;
            case DefendResolution::NONE: 
            default:
                damageTaken = std::max(1, currentCombatEnemy_.attack - playerStats_.defense);
                playerStats_.hp -= damageTaken;
                std::cout << currentCombatEnemy_.name << " memberikan " << damageTaken << " damage kepada Heismay." << std::endl;
                break;
        }
        playerStats_.hp = std::max(0, playerStats_.hp); 
        currentDefendOutcome_ = DefendResolution::NONE;
    }

    void checkCombatResult() { 
        if (currentCombatEnemy_.hp <= 0) {
            std::cout << "\n*** " << currentCombatEnemy_.name << " telah dikalahkan! ***" << std::endl;
            inCombat_ = false;
            currentSceneId_ = currentCombatEnemy_.victorySceneId;
            playerStats_.hp = playerStats_.maxHp; playerStats_.sp = playerStats_.maxSp;
            std::cout << "HP dan SP Heismay pulih sepenuhnya!" << std::endl;
        } else if (playerStats_.hp <= 0) {
            std::cout << "\n*** Heismay telah dikalahkan! ***" << std::endl;
            inCombat_ = false;
            handleEnding(currentCombatEnemy_.defeatEndingTitle); 
        }
    }
};

// Definisi NavigateCommand::execute() setelah GameEngine didefinisikan
void NavigateCommand::execute() { 
    gameEngine_.processNavigateChoice(choice_); 
}

//--------------------------------------------------------------------------------------------------
// Main Function
//--------------------------------------------------------------------------------------------------
int main() {
    GameEngine game;
    game.initializeGame();
    game.run();
    return 0;
}