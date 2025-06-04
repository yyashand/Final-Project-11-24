#include "GameDataTypes.h"
#include <utility> // Untuk std::move

SceneOption::SceneOption(std::string text, std::string nextId) 
    : optionText(std::move(text)), nextSceneId(std::move(nextId)) {}

Scene::Scene(std::string id, std::string desc, bool ending, std::string title, std::string enemyId)
    : sceneId(std::move(id)), 
      description(std::move(desc)), 
      isEnding(ending), 
      endingTitle(std::move(title)), 
      enemyIdToFightOnEnter(std::move(enemyId)) {}

void Scene::addOption(const std::string& optionChar, const std::string& optionDesc, const std::string& nextSceneIdSuffixOrFullId, bool isFullId) {
    if (isFullId) {
        options.emplace_back(optionChar + ". " + optionDesc, nextSceneIdSuffixOrFullId);
    } else {
        options.emplace_back(optionChar + ". " + optionDesc, this->sceneId + nextSceneIdSuffixOrFullId);
    }
}