// Isi file: NavigateCommand.h
#ifndef NAVIGATECOMMAND_H
#define NAVIGATECOMMAND_H

#include "Interfaces.h" // Untuk base class Command

class GameEngine; // Forward declaration

class NavigateCommand : public Command {
public:
    NavigateCommand(GameEngine& gameEngine, char choice);
    void execute() override;
private:
    GameEngine& gameEngine_; 
    char choice_;
};

#endif // NAVIGATECOMMAND_H