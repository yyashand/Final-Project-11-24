#ifndef CHOICELOGGEROBSERVER_H
#define CHOICELOGGEROBSERVER_H

#include "Interfaces.h" // Untuk Observer
#include <iostream>     // Untuk std::cout
#include <string>

class ChoiceLoggerObserver : public Observer {
public:
    void update(const std::string& choiceMade) override;
};

#endif // CHOICELOGGEROBSERVER_H