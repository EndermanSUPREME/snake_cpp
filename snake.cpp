#include "snake.hpp"

// global thread signal
std::atomic<bool> running(true);

// Linux function to fetch the key that is pressed
char GetKey() {
    termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);            // Save old settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);          // Disable canonical mode & echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);   // Apply new settings

    ch = getchar();                            // Read one character

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);   // Restore old settings
    return ch;
}

// thread target listens for keyboard input
// and uses it to move the player head around
void KeyListener(Snake* snake) {
    while (running) {
        char key = GetKey();

        // send only acceptable chars to player logic
        if (key == 'w' || key == 'a' ||key == 's' ||key == 'd')
            snake->ChangeDirection(key);
    }
}

int main() {
    // initialize game components
    Snake player;
    Game game(&player);
    
    // separate thread to listen to user input
    std::thread inputThread(KeyListener, &player);

    // main game thread
    while(game.Running()) {
        game.Update();
    }

    running = false;
    inputThread.join();
}
