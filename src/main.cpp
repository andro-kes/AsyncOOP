#include "game.h"
#include <iostream>

int main() {
    std::cout << "=== Laboratory Work #7: Asynchronous Programming ===\n";
    std::cout << "Variant 4: Wandering Knight\n";
    std::cout << "- Movement range: 30\n";
    std::cout << "- Kill range: 10\n";
    std::cout << "- Map size: 100x100\n";
    std::cout << "- NPCs: 50\n";
    std::cout << "- Duration: 30 seconds\n";
    std::cout << "- Threads: 3 (movement+combat detection, combat, map printing)\n\n";
    
    try {
        Game game(100, 50, 30);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
