#ifndef CONFIGURABLEENEMYFACTORY_H
#define CONFIGURABLEENEMYFACTORY_H

#include "Interfaces.h"     // Untuk IEnemyFactory
#include "GameDataTypes.h"  // Untuk Enemy
#include <map>
#include <string>
#include <stdexcept>       // Untuk std::runtime_error

class ConfigurableEnemyFactory : public IEnemyFactory {
private:
    std::map<std::string, Enemy> enemyBlueprints_; 
public:
    ConfigurableEnemyFactory(const std::map<std::string, Enemy>& blueprints);
    Enemy createEnemy(const std::string& enemyId) override;
};

#endif // CONFIGURABLEENEMYFACTORY_H