#ifndef INTERFACES_H
#define INTERFACES_H

#include <string>
#include <vector> // Untuk Subject
#include "GameDataTypes.h" // Karena IEnemyFactory mengembalikan Enemy

// Forward declaration jika diperlukan (di sini tidak terlalu mendesak)

class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& choiceMade) = 0;
};

class Subject {
public:
    virtual ~Subject() = default;
    void attach(Observer* observer);
    void detach(Observer* observer);
    void notify(const std::string& choiceMade);
private:
    std::vector<Observer*> observers_;
};

class IEnemyFactory {
public:
    virtual ~IEnemyFactory() = default;
    virtual Enemy createEnemy(const std::string& enemyId) = 0; 
};

class Command {
public:
    virtual ~Command() = default; 
    virtual void execute() = 0;
};

#endif // INTERFACES_H