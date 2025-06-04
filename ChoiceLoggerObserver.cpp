#include "ChoiceLoggerObserver.h"

void ChoiceLoggerObserver::update(const std::string& choiceMade) {
    std::cout << "\n--- [Notifikasi Observer] --- \nPilihan '" << choiceMade << "' (Non-Combat) telah dipilih.\n---------------------------\n";
}