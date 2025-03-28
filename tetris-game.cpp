#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <windows.h>
#include <conio.h>

enum class TetrominoType {
    I, O, T, S, Z, J, L
};

class Tetromino {
public:
    TetrominoType type;
    std::vector<std::vector<int>> shape;
    int color;

    Tetromino() : type(TetrominoType::I), color(0) {}

    Tetromino(TetrominoType t) : type(t) {
        initializeShape();
    }

private:
    void initializeShape() {
        switch(type) {
            case TetrominoType::I:
                shape = {{1,1,1,1}};
                color = 1;
                break;
            case TetrominoType::O:
                shape = {{1,1},{1,1}};
                color = 2;
                break;
            case TetrominoType::T:
                shape = {{0,1,0},{1,1,1}};
                color = 3;
                break;
            case TetrominoType::S:
                shape = {{0,1,1},{1,1,0}};
                color = 4;
                break;
            case TetrominoType::Z:
                shape = {{1,1,0},{0,1,1}};
                color = 5;
                break;
            case TetrominoType::J:
                shape = {{1,0,0},{1,1,1}};
                color = 6;
                break;
            case TetrominoType::L:
                shape = {{0,0,1},{1,1,1}};
                color = 7;
                break;
        }
    }
};

class TetrisGame {
private:
    static const int WIDTH = 10;
    static const int HEIGHT = 20;
    std::vector<std::vector<int>> grid;
    Tetromino currentPiece;
    int currentX, currentY;
    int score;
    int level;
    int dropTimer;
    bool isPaused;
    bool isGameFinished;
    bool shouldRestart;
    int pauseMenuSelection;
    HANDLE hConsole;
    COORD consoleBufferSize;
    SMALL_RECT windowSize;
    CHAR_INFO* consoleBuffer;

    TetrominoType getRandomTetromino() {
        return static_cast<TetrominoType>(rand() % 7);
    }

    bool canMove(const Tetromino& piece, int newX, int newY) {
        for (size_t y = 0; y < piece.shape.size(); ++y) {
            for (size_t x = 0; x < piece.shape[y].size(); ++x) {
                if (piece.shape[y][x]) {
                    int gridX = newX + x;
                    int gridY = newY + y;

                    // Check bounds
                    if (gridX < 0 || gridX >= WIDTH || 
                        gridY >= HEIGHT) {
                        return false;
                    }

                    // Check collision with existing blocks
                    if (gridY >= 0 && grid[gridY][gridX]) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

public:
    TetrisGame() : 
        grid(HEIGHT, std::vector<int>(WIDTH, 0)), 
        currentPiece(getRandomTetromino()),
        currentX(WIDTH / 2),
        currentY(0),
        score(0),
        level(1),
        dropTimer(0),
        isPaused(false),
        isGameFinished(false),
        shouldRestart(false),
        pauseMenuSelection(0) {
        
        srand(static_cast<unsigned>(time(nullptr)));
        
        // Prepare console for faster rendering
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        consoleBufferSize.X = (SHORT)((WIDTH * 2) + 20);
        consoleBufferSize.Y = (SHORT)(HEIGHT + 10);
        windowSize.Left = 0;
        windowSize.Top = 0;
        windowSize.Right = consoleBufferSize.X - 1;
        windowSize.Bottom = consoleBufferSize.Y - 1;
        
        SetConsoleScreenBufferSize(hConsole, consoleBufferSize);
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
        
        consoleBuffer = new CHAR_INFO[consoleBufferSize.X * consoleBufferSize.Y];
        
        // Hide the cursor
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    ~TetrisGame() {
        delete[] consoleBuffer;
    }

    bool run() {
        // Reset game state
        grid = std::vector<std::vector<int>>(HEIGHT, std::vector<int>(WIDTH, 0));
        currentPiece = Tetromino(getRandomTetromino());
        currentX = WIDTH / 2;
        currentY = 0;
        score = 0;
        level = 1;
        dropTimer = 0;
        isPaused = false;
        isGameFinished = false;
        shouldRestart = false;
        pauseMenuSelection = 0;

        while(!isGameOver()) {
            handleInput();
            
            if (!isPaused) {
                update();
            }
            
            render();
            
            // Handle restart request
            if (shouldRestart) {
                return true;
            }
            
            Sleep(50); // Faster drop speed
        }
        return gameOver();
    }

    void handleInput() {
        if (_kbhit()) {
            int key = _getch();
            if (isPaused) {
                // Pause menu navigation
                switch(key) {
                    case 72: // Up arrow
                        pauseMenuSelection = (pauseMenuSelection - 1 + 3) % 3;
                        render();
                        break;
                    case 80: // Down arrow
                        pauseMenuSelection = (pauseMenuSelection + 1) % 3;
                        render();
                        break;
                    case 13: // Enter key
                        handlePauseMenuSelection();
                        break;
                }
            }
            else {
                // Regular game controls
                switch(key) {
                    // Arrow keys
                    case 75: // Left arrow
                    case 'a':
                    case 'A':
                        moveLeft();
                        break;
                    case 77: // Right arrow
                    case 'd':
                    case 'D':
                        moveRight();
                        break;
                    case 72: // Up arrow
                    case 'w':
                    case 'W':
                        rotate();
                        break;
                    case 80: // Down arrow
                    case 's':
                    case 'S':
                        softDrop();
                        break;
                    
                    // Spacebar for hard drop
                    case 32:
                        hardDrop();
                        break;
                    
                    // Escape key for pause
                    case 27: // ESC key
                        togglePause();
                        break;
                }
            }
        }
    }

    void handlePauseMenuSelection() {
        switch(pauseMenuSelection) {
            case 0: // Continue
                togglePause();
                break;
            case 1: // Restart
                shouldRestart = true;
                isPaused = false;
                break;
            case 2: // Exit
                isGameFinished = true;
                shouldRestart = false;
                return;
        }
    }

    void togglePause() {
        isPaused = !isPaused;
        pauseMenuSelection = 0;
        render(); // Render pause state
    }

    void moveLeft() {
        if (canMove(currentPiece, currentX - 1, currentY)) {
            currentX--;
        }
    }

    void moveRight() {
        if (canMove(currentPiece, currentX + 1, currentY)) {
            currentX++;
        }
    }

    void rotate() {
        Tetromino rotatedPiece = currentPiece;
        // Rotate 90 degrees
        std::vector<std::vector<int>> newShape(
            currentPiece.shape[0].size(), 
            std::vector<int>(currentPiece.shape.size())
        );
        
        for (size_t i = 0; i < currentPiece.shape.size(); ++i) {
            for (size_t j = 0; j < currentPiece.shape[i].size(); ++j) {
                newShape[j][currentPiece.shape.size() - 1 - i] = currentPiece.shape[i][j];
            }
        }
        
        rotatedPiece.shape = newShape;
        
        if (canMove(rotatedPiece, currentX, currentY)) {
            currentPiece = rotatedPiece;
        }
    }

    void update() {
        dropTimer++;
        
        // Adjust drop speed based on level
        if (dropTimer >= (20 - level)) {
            if (!canMove(currentPiece, currentX, currentY + 1)) {
                lockPiece();
                clearLines();
                spawnNewPiece();
            } else {
                currentY++;
            }
            dropTimer = 0;
        }
    }

    void softDrop() {
        if (canMove(currentPiece, currentX, currentY + 1)) {
            currentY++;
        } else {
            lockPiece();
            clearLines();
            spawnNewPiece();
        }
    }

    void hardDrop() {
        while (canMove(currentPiece, currentX, currentY + 1)) {
            currentY++;
        }
        lockPiece();
        clearLines();
        spawnNewPiece();
    }

    void lockPiece() {
        for (size_t y = 0; y < currentPiece.shape.size(); ++y) {
            for (size_t x = 0; x < currentPiece.shape[y].size(); ++x) {
                if (currentPiece.shape[y][x]) {
                    grid[currentY + y][currentX + x] = currentPiece.color;
                }
            }
        }
    }

    void clearLines() {
        int linesCleared = 0;
        for (int y = HEIGHT - 1; y >= 0; --y) {
            bool fullLine = true;
            for (int x = 0; x < WIDTH; ++x) {
                if (!grid[y][x]) {
                    fullLine = false;
                    break;
                }
            }

            if (fullLine) {
                // Remove the line
                grid.erase(grid.begin() + y);
                // Add a new empty line at the top
                grid.insert(grid.begin(), std::vector<int>(WIDTH, 0));
                linesCleared++;
                y++; // Check the same row again
            }
        }

        // Update score and level
        updateScore(linesCleared);
    }

    void updateScore(int linesCleared) {
        switch(linesCleared) {
            case 1: score += 100; break;
            case 2: score += 300; break;
            case 3: score += 500; break;
            case 4: score += 800; break;
        }

        // Increase level every 10 lines
        level = 1 + (score / 1000);
    }

    void spawnNewPiece() {
        currentPiece = Tetromino(getRandomTetromino());
        currentX = WIDTH / 2;
        currentY = 0;

        // Check if game is over
        if (!canMove(currentPiece, currentX, currentY)) {
            gameOver();
        }
    }

    bool isGameOver() {
        // Check if any block reached the top
        for (int x = 0; x < WIDTH; ++x) {
            if (grid[0][x]) {
                return true;
            }
        }
        return false;
    }

    bool gameOver() {
        isGameFinished = true;
        
        // Clear console buffer
        ZeroMemory(consoleBuffer, consoleBufferSize.X * consoleBufferSize.Y * sizeof(CHAR_INFO));
        
        // Write game over message
        writeStringToBuffer(0, 0, "GAME OVER!");
        std::string scoreMsg = "Final Score: " + std::to_string(score);
        writeStringToBuffer(0, 1, scoreMsg);
        writeStringToBuffer(0, 2, "Press R to Replay or ESC to Exit");

        // Update console
        COORD bufferCoord = {0, 0};
        WriteConsoleOutput(hConsole, consoleBuffer, consoleBufferSize, bufferCoord, &windowSize);

        // Wait for user input
        while (true) {
            if (_kbhit()) {
                int key = _getch();
                if (key == 'r' || key == 'R') {
                    return true; // Replay
                }
                if (key == 27) { // ESC
                    return false; // Exit
                }
            }
            Sleep(50);
        }
    }

    void render() {
        // Clear console buffer
        ZeroMemory(consoleBuffer, consoleBufferSize.X * consoleBufferSize.Y * sizeof(CHAR_INFO));
        
        // Render game info
        writeStringToBuffer(0, 0, "TETRIS GAME");
        writeStringToBuffer(0, 1, "Score: " + std::to_string(score) + " | Level: " + std::to_string(level));
        writeStringToBuffer(0, 2, "----------------------");

        // Render grid
        for (int y = 0; y < HEIGHT; ++y) {
            writeStringToBuffer(0, y + 3, "|");
            for (int x = 0; x < WIDTH; ++x) {
                bool pieceHere = false;
                
                // Check if current piece is at this position
                for (size_t py = 0; py < currentPiece.shape.size(); ++py) {
                    for (size_t px = 0; px < currentPiece.shape[py].size(); ++px) {
                        if (currentPiece.shape[py][px] && 
                            y == currentY + py && 
                            x == currentX + px) {
                            pieceHere = true;
                            break;
                        }
                    }
                    if (pieceHere) break;
                }

                // Render only current and locked pieces
                if (pieceHere) {
                    writeStringToBuffer(1 + x * 2, y + 3, "[]");
                } else if (grid[y][x]) {
                    writeStringToBuffer(1 + x * 2, y + 3, "[]");
                }
            }
            writeStringToBuffer(1 + WIDTH * 2, y + 3, "|");
        }
        writeStringToBuffer(0, HEIGHT + 3, "----------------------");

        // Render pause menu
        if (isPaused) {
            writeStringToBuffer(0, HEIGHT + 4, "PAUSE MENU:");
            writeStringToBuffer(0, HEIGHT + 5, pauseMenuSelection == 0 ? "> Continue" : "  Continue");
            writeStringToBuffer(0, HEIGHT + 6, pauseMenuSelection == 1 ? "> Restart" : "  Restart");
            writeStringToBuffer(0, HEIGHT + 7, pauseMenuSelection == 2 ? "> Exit" : "  Exit");
            writeStringToBuffer(0, HEIGHT + 8, "Use arrows and Enter to select");
        }

        // Update console
        COORD bufferCoord = {0, 0};
        WriteConsoleOutput(hConsole, consoleBuffer, consoleBufferSize, bufferCoord, &windowSize);
    }

    void writeStringToBuffer(int x, int y, const std::string& text) {
        for (size_t i = 0; i < text.length(); ++i) {
            consoleBuffer[y * consoleBufferSize.X + x + i].Char.AsciiChar = text[i];
            consoleBuffer[y * consoleBufferSize.X + x + i].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    }
};

int main() {
    while (true) {
        TetrisGame game;
        if (!game.run()) {
            break; // Exit the game if run() returns false
        }
    }
    return 0;
}
