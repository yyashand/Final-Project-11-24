#include "MapsCommand.h"
#include "GameEngine.h" // Butuh definisi GameEngine lengkap

NavigateCommand::NavigateCommand(GameEngine& gameEngine, char choice)
    : gameEngine_(gameEngine), choice_(choice) {}

void NavigateCommand::execute() { 
    gameEngine_.processNavigateChoice(choice_); 
}