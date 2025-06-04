#include "Interfaces.h" // Berisi deklarasi Subject dan Observer
#include <algorithm>   // Untuk std::find

void Subject::attach(Observer* observer) {
    observers_.push_back(observer);
}

void Subject::detach(Observer* observer) {
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

void Subject::notify(const std::string& choiceMade) {
    for (Observer* observer : observers_) {
        observer->update(choiceMade);
    }
}