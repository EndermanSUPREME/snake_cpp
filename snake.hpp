#ifndef SNAKE
#define SNAKE

#include <iostream>
#include <string> // ANSI color escape strings
#include <vector>
#include <cstdlib> // allow screen clear
#include <unistd.h> // python-like sleep function

#include <thread>
#include <atomic>
#include <termios.h>

#include <ctime>
#include <random>

enum class Direction { UP, DOWN, LEFT, RIGHT };

class Snake {
public:
    Snake(): length(1), alive(true){}
    void Die() {
        alive = false;
    }
    bool Alive() { return alive; }
    int GetHeadRow() { return headPos[0]; }
    int GetHeadCol() { return headPos[1]; }
    // return as a non-mutable reference due to pointers
    std::vector<int*>* GetSegments() { return &segments; }

    // Extern function to allow changing of direction
    // (used to prevent weird movement between frames
    void ReadyForMove() { ready = true; }
    bool UserChangeDir() { return ready; }

    // Head position changes before map redraw
    void UpdateHeadPosition(bool userChanged = false) {
        if (!alive) return;

        // let the game update thread move the snake
        if (userChanged) {
            ready = false;
            return;
        }

        // used when generating new segments and updating
        // how their drawn on the terminal
        UpdatePrev(headPos[0], headPos[1]);

        switch (dir) {
            case Direction::UP: // row - 1
                --headPos[0];
            break;
            case Direction::DOWN: // row + 1
                ++headPos[0];
            break;
            case Direction::LEFT: // col - 1
                --headPos[1];
            break;
            case Direction::RIGHT: // col + 1
                ++headPos[1];
            break;
        }

        UpdateSegments();
    }

    void UpdatePrev(int r, int c) {
        prevPos[0] = r;
        prevPos[1] = c;
    }

    void UpdateSegments() {
        if (segments.size() == 0) return;

        for (size_t i = 0; i < segments.size(); ++i) {
            // track current segment position before change
            int prevRow = segments[i][0];
            int prevCol = segments[i][1];

            // change segment position
            segments[i][0] = prevPos[0];
            segments[i][1] = prevPos[1];

            // update prev and descend down the snake
            UpdatePrev(prevRow, prevCol);
        }
    }

    // executed on separate thread listening for user-input
    void ChangeDirection(char c) {
        if (!alive) return;

        switch (dir) {
            case Direction::UP:
                if (c == 'a') {
                    dir = Direction::LEFT;
                    UpdateHeadPosition(true);
                } else if (c == 'd') {
                    dir = Direction::RIGHT;
                    UpdateHeadPosition(true);
                }
            break;
            case Direction::DOWN:
                if (c == 'a') {
                    dir = Direction::LEFT;
                    UpdateHeadPosition(true);
                } else if (c == 'd') {
                    dir = Direction::RIGHT;
                    UpdateHeadPosition(true);
                }
            break;
            case Direction::LEFT:
                if (c == 'w') {
                    dir = Direction::UP;
                    UpdateHeadPosition(true);
                } else if (c == 's') {
                    dir = Direction::DOWN;
                    UpdateHeadPosition(true);
                }
            break;
            case Direction::RIGHT:
                if (c == 'w') {
                    dir = Direction::UP;
                    UpdateHeadPosition(true);
                } else if (c == 's') {
                    dir = Direction::DOWN;
                    UpdateHeadPosition(true);
                }
            break;
        }
    }

    void GrowSegment() {
        segments.push_back(new int[2]{-1,-1});
    }

    void PrintMovementInfo() {
        switch (dir) {
            case Direction::UP:
                std::cout << "Movement: UP" << std::endl;
            break;
            case Direction::DOWN:
                std::cout << "Movement: DOWN" << std::endl;
            break;
            case Direction::LEFT:
                std::cout << "Movement: LEFT" << std::endl;
            break;
            case Direction::RIGHT:
                std::cout << "Movement: RIGHT" << std::endl;
            break;
        }

        std::cout << "HEAD POSITION: ROW -> " << headPos[0] <<
        " | COL -> " << headPos[1] << std::endl;

        if (segments.size() > 0) {
            std::cout << "SEGMENT POSITION: ROW -> " << segments[0][0] <<
            " | COL -> " << segments[0][1] << std::endl;
        }
    }
private:
    int length;
    bool alive;
    bool ready = false;
    Direction dir = Direction::DOWN;
    int headPos[2] = {8, 6}; // r, c
    int prevPos[2]{0,0};
    std::vector<int*> segments;
};

class Game {
public:
    Game(Snake* s, int l = 30): snake(s), length(30){
        srand(time(nullptr));
        GenerateApple();
    }
    
    bool Running() { return !gameOver; }
    bool GameWon() { return gameWon; }
    
    // Snake game update function
    void Update() {
        if (gameWon) {
            gameOver = true;
            
            ClearScreen();

            std::cout << "\033[38;5;76m" << "YOU WIN!!!" << "\033[0m" << std::endl;
            std::cout << "=== PRESS ANY KEY TO CONTINUE ===" << std::endl;
            
            return;
        }

        if (!snake->Alive()) {
            gameOver = true;

            ClearScreen();

            std::cout << "\033[38;5;160m" << "Snek Di" << "\033[0m" << std::endl;
            std::cout << "=== PRESS ANY KEY TO CONTINUE ===" << std::endl;

            return;
        }

        // auto-movement when player doesnt change dir
        snake->UpdateHeadPosition();
        Draw();
        sleep(1); // sleeps for N seconds
    }

private:
    void ClearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
    void GenerateApple() {
        if (applePos != nullptr) delete applePos;

        int min = 4;
        int max = length - 4;

        applePos = new int[2]{0,0};
        bool invalidPos = true;
        while(invalidPos) {
            // offset subtraction prevents apples from spawning in walls
            applePos[0] = min + rand() % (max - min + 1);
            applePos[1] = min + rand() % (max - min + 1);

            bool appleInSeg = false;
            for (const int* seg : *(snake->GetSegments())) {
                if (applePos[0] == seg[0] && applePos[1] == seg[1]) {
                    appleInSeg = true;
                    break;
                }
            }
            if (!appleInSeg) invalidPos = false;
        }
    }
    void Draw() {
        ClearScreen();

        std::cout << "Apple Position: (" << applePos[0] << ", " << applePos[1] << ")" << std::endl;
        std::cout << "Score: " << "\033[38;5;118m" << score << "\033[0m" << std::endl;

        // DEBUGGING
        // snake->PrintMovementInfo();

        // draw map border
        int rowLen = length;
        int colLen = length;
        for (int row = 0; row < rowLen; ++row) {
            for (int col = 0; col < colLen; ++col) {
                if (row == 0 || row == rowLen-1) {
                    // check for snake head wall collision
                    if (row == snake->GetHeadRow()) {
                        snake->Die();
                    }
                    // draw top and bottom borders
                    std::cout << wall << ' ';
                }

                if (row > 0 && row < rowLen-1) {
                    if (col == 0 || col == colLen-1) {
                        // check for snake head wall collision
                        if (col == snake->GetHeadCol()) {
                            snake->Die();
                        }
                        // draw the left and right walls
                        std::cout << wall << ' ';
                    } else {
                        // draw inside area
                        if (row == snake->GetHeadRow() && col == snake->GetHeadCol()) {
                            // draw snake head
                            std::cout << 'O';

                            // detect self-collision
                            for (const int* seg : *(snake->GetSegments())) {
                                int segRow = seg[0];
                                int segCol = seg[1];

                                if (snake->GetHeadRow() == segRow &&
                                    snake->GetHeadCol() == segCol) {
                                    snake->Die();
                                }
                            }
                        } else {
                            bool drawAir = true;

                            // draw snake segment
                            for (const int* seg : *(snake->GetSegments())) {
                                int segRow = seg[0];
                                int segCol = seg[1];

                                if (row == segRow && col == segCol) {
                                    std::cout << "\033[32m" << 'O' << "\033[0m";
                                    drawAir = false;
                                    break;
                                }
                            }

                            // Draw the apple
                            if (row == applePos[0] && col == applePos[1]) {
                                std::cout << "\033[31m" << 'A' << "\033[0m";
                                drawAir = false;
                            }

                            if (drawAir) {
                                // empty area
                                std::cout << ' ';
                            }
                        }

                        // check for snake head apple collection
                        if (applePos[0] == snake->GetHeadRow() && applePos[1] == snake->GetHeadCol()) {
                            ++score;
                            GenerateApple();
                            snake->GrowSegment();
                        }

                        // buffer space
                        std::cout << ' ';
                    }
                }
            }
            std::cout << std::endl;
        }

        // after each rendered frame allow the player to change direction
        snake->ReadyForMove();
    }

    void CheckProgress() {
        // if the score is equal to the geometric area
        // of the play area
        if (score >= (length-1) * (length-1)) {
            gameWon = true;
        }
    }

    int length;
    int score = 0;
    char wall = '#';
    Snake* snake;
    bool gameOver = false;
    bool gameWon = false;
    int* applePos = nullptr;
};

#endif
