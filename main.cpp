#include "GameEngine.h" 
// Tidak perlu #include <iostream> jika tidak ada output langsung di sini
// Tidak perlu srand() di sini jika sudah ada di konstruktor GameEngine

int main() {
    GameEngine game;
    game.initializeGame();
    game.run();
    return 0;
}