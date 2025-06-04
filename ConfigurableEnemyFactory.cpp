#include "ConfigurableEnemyFactory.h"

ConfigurableEnemyFactory::ConfigurableEnemyFactory(const std::map<std::string, Enemy>& blueprints)
    : enemyBlueprints_(blueprints) {}

Enemy ConfigurableEnemyFactory::createEnemy(const std::string& enemyId) {
    if (enemyBlueprints_.count(enemyId)) {
        return enemyBlueprints_.at(enemyId); 
    }
    throw std::runtime_error("Enemy blueprint not found in factory: " + enemyId);
}