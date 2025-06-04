#ifndef ENDINGTRACKER_H
#define ENDINGTRACKER_H

#include <string>
#include <set>
#include <iostream> // Untuk std::cout

class EndingTracker {
public:
    static EndingTracker& getInstance() { 
        static EndingTracker instance; 
        return instance; 
    }
    EndingTracker(const EndingTracker&) = delete; 
    void operator=(const EndingTracker&) = delete;
    
    void recordEnding(const std::string& endingTitle) { 
        if (!endingTitle.empty()) unlockedEndings_.insert(endingTitle); 
    }
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
    std::set<std::string> unlockedEndings_; 
    int totalPossibleEndings_;
};

#endif // ENDINGTRACKER_H